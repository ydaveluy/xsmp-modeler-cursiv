// Opening an SMP XML file as XSMP: the commands, the default editor and the
// virtual documents that mirror the XML files.
import assert from 'node:assert/strict';
import fs from 'node:fs';
import { test } from 'node:test';
import { URI } from 'vscode-uri';
import { createTypeScriptLoader, src } from './typescript-loader.mjs';
import { runtime } from './vscode-mock.mjs';

const plain = value => JSON.parse(JSON.stringify(value));
const metadata = JSON.parse(fs.readFileSync(new URL('../package.json', import.meta.url), 'utf8'));

test('SMP commands open a mirror and reopen its XML source', async t => {
    const { state, context, client, modules } = runtime(t);
    modules(src('workflows.ts')).registerWorkflows(context, () => client);
    await state.commands.get('xsmp.openSmpMirror')(URI.file('/workspace/demo/test.smpcat'));
    assert.deepEqual(plain(state.requests), [['workspace/textDocumentContent', { uri: 'xsmp-smp:/workspace/demo/test.smpcat' }]]);
    assert.equal(state.opened[0].uri.toString(), 'xsmp-smp:/workspace/demo/test.smpcat'); assert.deepEqual(plain(state.shown[0].options), { preview: false });
    await state.commands.get('xsmp.openSmpXmlSource')(URI.parse('xsmp-smp:/workspace/demo/test.smpcat'));
    assert.equal(state.opened[1].uri.toString(), 'file:///workspace/demo/test.smpcat'); assert.equal(state.shown[1].document.uri.toString(), 'file:///workspace/demo/test.smpcat'); assert.deepEqual(plain(state.shown[1].options), { preview: false }); assert.deepEqual(state.errors, []); assert.deepEqual(state.info, []);
});

test('invalid mirror targets and absent content keep their user-facing messages', async t => {
    const { state, context, client, modules } = runtime(t, undefined);
    client.sendRequest = async () => undefined;
    modules(src('workflows.ts')).registerWorkflows(context, () => client);
    await state.commands.get('xsmp.openSmpMirror')(); await state.commands.get('xsmp.openSmpMirror')(URI.file('/workspace/demo/test.smpcat')); await state.commands.get('xsmp.openSmpXmlSource')();
    assert.deepEqual(state.errors, ['Open or select an SMP XML file (.smpcat, .smpcfg, .smplnk, .smpasb, or .smpsed) before opening XSMP.', 'Open an SMP XML mirror or source file before reopening the XML source.']);
    assert.deepEqual(state.info, ["No XSMP view is currently available for '/workspace/demo/test.smpcat'."]);
});

test('the custom preview requests the mirror content, opens it, and closes only its matching tab', async t => {
    const { state, vscode, context, client, modules } = runtime(t), preview = modules(src('smp-mirror-preview.ts'));
    preview.registerSmpMirrorPreview(context, () => client);
    const provider = state.providers.get(preview.SmpMirrorPreviewViewType).provider, uri = URI.file('/workspace/demo/test.smpcat');
    const tab = { input: new vscode.TabInputCustom(uri, preview.SmpMirrorPreviewViewType) }; state.tabs = [{ tabs: [tab] }];
    const panel = { viewColumn: 2, webview: { html: '' } }; await provider.resolveCustomEditor(await provider.openCustomDocument(uri), panel);
    assert(panel.webview.html.includes('Opening XSMP for /workspace/demo/test.smpcat...'));
    assert.deepEqual(plain(state.requests), [['workspace/textDocumentContent', { uri: 'xsmp-smp:/workspace/demo/test.smpcat' }]]);
    assert.equal(state.opened[0].uri.toString(), 'xsmp-smp:/workspace/demo/test.smpcat');
    assert.deepEqual(plain(state.shown[0].options), { viewColumn: 2, preview: true, preserveFocus: true }); assert.deepEqual(state.closed, [{ tab, preserveFocus: true }]);
});

test('contribution metadata: SMP title commands, readonly activation and default editor', () => {
    for (const [command, title] of [['xsmp.openSmpMirror', 'Open XSMP Preview'], ['xsmp.openSmpXmlSource', 'Open XML Source']]) assert(metadata.contributes.commands.some(x => x.command === command && x.title === title));
    assert.deepEqual(metadata.contributes.menus['editor/title'], [{ command: 'xsmp.openSmpMirror', group: 'navigation@100', when: 'resourceScheme == file && (resourceExtname == .smpcat || resourceExtname == .smpcfg || resourceExtname == .smplnk || resourceExtname == .smpasb || resourceExtname == .smpsed)' }, { command: 'xsmp.openSmpXmlSource', group: 'navigation@101', when: 'resourceScheme == xsmp-smp' }]);
    for (const event of ['onFileSystem:xsmp', 'onFileSystem:xsmp-smp']) assert(metadata.activationEvents.includes(event));
    assert.deepEqual(metadata.contributes.customEditors, [{ viewType: 'xsmp.smpMirrorPreview', displayName: 'XSMP Preview', priority: 'default', selector: ['smpcat', 'smpcfg', 'smplnk', 'smpasb', 'smpsed'].map(extension => ({ filenamePattern: `file:/**/*.${extension}` })) }]);
});

test('an SMP XML file maps to its virtual XSMP document, and no other document does', () => {
    const helper = createTypeScriptLoader({ vscode: { Uri: URI } })(src('smp-mirror-preview-support.ts'));
    for (const [source, mirror] of [['file:///workspace/demo/test.smpcat', 'xsmp-smp:/workspace/demo/test.smpcat'], ['file:///workspace/demo/test.smpsed', 'xsmp-smp:/workspace/demo/test.smpsed']]) assert.equal(helper.smpMirrorUri(URI.parse(source)).toString(), mirror);
    for (const other of ['file:///workspace/demo/test.xsmpcat', 'xsmp-smp:/workspace/demo/test.smpcat']) assert.equal(helper.smpMirrorUri(URI.parse(other)), undefined);
});

test('an XSMP file beside the XML wins over the virtual document, for the command and the preview', async t => {
    const { state, vscode, context, client, modules } = runtime(t), preview = modules(src('smp-mirror-preview.ts')), source = URI.file('/workspace/demo/test.smpcat');
    vscode.workspace.fs.stat = async () => ({});
    modules(src('workflows.ts')).registerWorkflows(context, () => client);
    preview.registerSmpMirrorPreview(context, () => client);
    await state.commands.get('xsmp.openSmpMirror')(source);
    await state.providers.get(preview.SmpMirrorPreviewViewType).provider.resolveCustomEditor({ uri: source }, { viewColumn: 1, webview: { html: '' } });
    assert.deepEqual(state.opened.map(document => document.uri.toString()), ['file:///workspace/demo/test.xsmpcat', 'file:///workspace/demo/test.xsmpcat']); assert.deepEqual(state.requests, []);
});

test('without an XSMP view the preview falls back on the XML file', async t => {
    const { state, context, client, modules } = runtime(t, undefined), preview = modules(src('smp-mirror-preview.ts')), source = URI.file('/workspace/demo/test.smpcat');
    client.sendRequest = async () => undefined;
    preview.registerSmpMirrorPreview(context, () => client);
    await state.providers.get(preview.SmpMirrorPreviewViewType).provider.resolveCustomEditor({ uri: source }, { viewColumn: 1, webview: { html: '' } });
    assert.equal(state.opened[0].uri.toString(), source.toString());
});

test('reopening a mirror resolves the XML file without virtual query or fragment', () => {
    const helper = createTypeScriptLoader({ vscode: { Uri: URI } })(src('workflows.ts'));
    for (const [input, expected] of [
        ['xsmp-smp:/workspace/test.xsmpcat', 'file:///workspace/test.smpcat'],
        ['xsmp-smp:/workspace/a%23b%3Fc%25%C3%A9.xsmpcfg', 'file:///workspace/a%23b%3Fc%25%C3%A9.smpcfg'],
        ['xsmp-smp:/workspace/a.xsmpcat?session=1#L2', 'file:///workspace/a.smpcat'],
        ['xsmp-smp:/workspace/a.xsmpsed#task', 'file:///workspace/a.smpsed'],
        ['xsmp-smp:/workspace/test.smpcat', 'file:///workspace/test.smpcat'],
        ['xsmp-smp:/workspace/a.smpcfg?session=1#L2', 'file:///workspace/a.smpcfg'],
    ]) assert.equal(helper.resolveImportSmpTarget(URI.parse(input)).toString(), expected);
});

test('a mirror takes the language of its kind, and a mirror of an earlier version gives way to it', async t => {
    const { state, vscode, context, client, modules } = runtime(t), preview = modules(src('smp-mirror-preview.ts'));
    const mirror = { uri: URI.parse('xsmp-smp:/workspace/demo/test.smpcfg'), languageId: 'xml', isClosed: false };
    state.documents = [mirror, { uri: URI.file('/workspace/demo/test.smpcfg'), languageId: 'xml', isClosed: false }];
    const legacy = URI.parse('xsmp-smp:/workspace/demo/old.xsmpcat');
    const group = { viewColumn: 3, tabs: [] }, tab = { input: new vscode.TabInputText(legacy), group, isPreview: true };
    group.tabs.push(tab); state.tabs = [group];
    preview.registerSmpMirrorPreview(context, () => client);
    await new Promise(resolve => setImmediate(resolve));
    assert.deepEqual(state.languages, [['xsmp-smp:/workspace/demo/test.smpcfg', 'xsmpcfg']]);
    assert.equal(state.opened[0].uri.toString(), 'xsmp-smp:/workspace/demo/old.smpcat');
    assert.deepEqual(plain(state.shown[0].options), { viewColumn: 3, preserveFocus: true, preview: true });
    assert.deepEqual(state.closed, [{ tab, preserveFocus: true }]);
});
