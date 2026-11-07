// Starting, stopping and restarting the language client.
import assert from 'node:assert/strict';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { test } from 'node:test';
import { URI } from 'vscode-uri';
import { createTypeScriptLoader, src } from './typescript-loader.mjs';

const tick = () => new Promise(resolve => setTimeout(resolve, 0));
async function until(check) { for (let i = 0; i < 100 && !check(); i++) await tick(); assert(check()); }

const Stopped = 1, Running = 2, Starting = 3;

// The extension with a fake client. `contributions` lists what the server is
// asked to register, `request` answers these requests and `start` starts the client.
function harness(t, { contributions = [], request = async () => ({ registered: [], failures: [] }), start = async client => { client.state = Running; } } = {}) {
    const directory = fs.mkdtempSync(path.join(os.tmpdir(), 'xsmp-client-'));
    t.after(() => fs.rmSync(directory, { recursive: true, force: true }));
    fs.writeFileSync(path.join(directory, 'native'), Buffer.from([0x7f, 0x45, 0x4c, 0x46]));
    const state = { stops: 0, listeners: new Set(), logs: [], requests: 0 };
    class Client {
        state = Stopped;
        constructor(_id, _name, _server, options) { state.client = this; state.options = options; }
        start() { return start(this); }
        async stop() { state.stops++; this.state = Stopped; }
        onDidChangeState(callback) { state.listeners.add(callback); return { dispose() { state.listeners.delete(callback); } }; }
        sendRequest() { state.requests++; return request(); }
    }
    const channel = { appendLine: line => state.logs.push(line), show: () => state.logs.push('show'), dispose() {} };
    const vscode = {
        Uri: URI, EventEmitter: class { event = () => ({ dispose() {} }); },
        extensions: { all: contributions.length ? [{ id: 'vendor.tool', extensionPath: directory, packageJSON: { contributes: { xsmp: contributions } } }] : [] },
        commands: { registerCommand: () => ({ dispose() {} }) },
        window: { createOutputChannel: () => channel, showErrorMessage: assert.fail },
        workspace: { getConfiguration: () => ({ get: () => path.join(directory, 'native') }), createFileSystemWatcher: () => ({ dispose() {} }), registerFileSystemProvider: () => ({ dispose() {} }) },
    };
    const extension = createTypeScriptLoader({
        vscode, 'vscode-languageclient/node': { State: { Stopped, Running, Starting }, LanguageClient: Client },
        './mcp': { startMcpBridge: () => ({ dispose() {} }) }, './workflows': { registerWorkflows() {} }, './embedded-documentation': { registerEmbeddedDocumentation() {} }, './smp-mirror-preview': { registerSmpMirrorPreview() {} },
    })(src('extension.ts'));
    return { extension, state, context: { extensionPath: directory, extension: { id: 'xsmpcat.modeler' }, subscriptions: [] } };
}
const tool = { descriptor: 'tool.xsmptool', handler: 'handler.mjs', apiVersion: '^2' };

test('a failed start does not stop the inactive client', async t => {
    const error = new Error('startup failed');
    const { extension, state, context } = harness(t, { start: async () => { throw error; } });
    await assert.rejects(extension.activate(context), caught => caught === error);
    assert.equal(state.stops, 0); assert(state.logs.some(line => line.includes('Failed to activate XSMP extension')));
    await extension.deactivate(); assert.equal(state.stops, 0);
});

test('a failed registration stops the server, leaves no listener and resets the extension client', async t => {
    const { extension, state, context } = harness(t, { contributions: [tool], request: async () => { throw new Error('registration failed'); } });
    await assert.rejects(extension.activate(context), /registration failed/);
    assert.equal(state.stops, 1); assert.equal(state.listeners.size, 0);
    assert(state.logs.some(line => line.includes('Failed to activate XSMP extension')));
    await extension.deactivate(); assert.equal(state.stops, 1);
});

test('an activation failure waits for a concurrent restart and keeps its original error', async t => {
    const error = new Error('activation failed'), stopError = new Error('stop failed'); let fail, finish;
    const { extension, state, context } = harness(t, { contributions: [tool], request: () => new Promise((_resolve, reject) => { fail = reject; }) });
    const activation = extension.activate(context);
    await until(() => fail);
    state.client.state = Starting; state.client.start = () => new Promise(resolve => { finish = resolve; });
    state.client.stop = async () => { state.stops++; throw stopError; };
    fail(error); await until(() => finish); assert.equal(state.stops, 0);
    state.client.state = Running; finish();
    await assert.rejects(activation, caught => caught === error);
    assert.equal(state.stops, 1); assert(state.logs.some(line => line.includes('Failed to stop the XSMP client') && line.includes('stop failed')));
});

test('a restarted server registers the contributions again and a failure is only logged', async t => {
    let answer = async () => ({ registered: [], failures: [] });
    const { extension, state, context } = harness(t, { contributions: [tool], request: () => answer() });
    await extension.activate(context);
    const [listener] = state.listeners; assert.equal(state.requests, 1);
    listener({ newState: Starting }); assert.equal(state.requests, 1);
    listener({ newState: Running }); assert.equal(state.requests, 2);
    answer = async () => { throw new Error('restart failed'); };
    listener({ newState: Running }); await until(() => state.logs.some(line => line.includes('Failed to restore external XSMP contributions') && line.includes('restart failed')));
    assert.equal(state.stops, 0);
    for (const subscription of context.subscriptions) subscription.dispose?.(); assert.equal(state.listeners.size, 0);
    await extension.deactivate();
});

test('deactivation waits for a concurrent client restart before stopping', async t => {
    const { extension, state, context } = harness(t);
    await extension.activate(context);
    let finish; const restarting = new Promise(resolve => finish = resolve);
    state.client.state = Starting; state.client.start = () => restarting;
    const stopped = extension.deactivate(); await Promise.resolve(); assert.equal(state.stops, 0);
    state.client.state = Running; finish(); await stopped; assert.equal(state.stops, 1);
});

test('deactivation gives up on a restart that fails', async t => {
    const { extension, state, context } = harness(t);
    await extension.activate(context);
    state.client.state = Starting; state.client.start = async () => { state.client.state = Stopped; throw new Error('startup failed'); };
    await extension.deactivate(); assert.equal(state.stops, 0);
});

test('language-server documentation cannot trust arbitrary command links from workspace comments', async t => {
    const { extension, state, context } = harness(t);
    await extension.activate(context);
    assert.equal(state.options.markdown.isTrusted, false);
    await extension.deactivate();
});
