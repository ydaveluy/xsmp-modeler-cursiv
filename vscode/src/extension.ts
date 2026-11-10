import { registerSmpMirrorPreview } from './smp-mirror-preview';
import { registerWorkflows } from './workflows';
import { registerEmbeddedDocumentation } from './embedded-documentation';
import { createXsmpDocumentSelector } from './xsmp-language-support';
import * as fs from 'node:fs';
import * as path from 'node:path';
import * as vscode from 'vscode';
import type { LanguageClientOptions, ServerOptions } from 'vscode-languageclient/node';
import { LanguageClient, State } from 'vscode-languageclient/node';
import { startMcpBridge } from './mcp';
import { discoverXsmpContributions, registerDiscoveredXsmpContributions } from './contributions/register';

let client: LanguageClient | undefined;
let outputChannel: vscode.LogOutputChannel | undefined;

export function resolveServerCommand(context: vscode.ExtensionContext): string | undefined {
    const configured = vscode.workspace
        .getConfiguration()
        .get<string>('xsmp.server.path');
    if (configured && configured.trim()) {
        return configured.trim();
    }

    const executable = process.platform === 'win32' ? 'xsmp_lsp.exe' : 'xsmp_lsp';

    // The F5 launch configuration sets CURSIV_SERVER_DIR to the build folder of
    // the active CMake preset. A WASI build names the server `xsmp_lsp` on every
    // host, including Windows.
    const presetDirectory = process.env.CURSIV_SERVER_DIR?.trim();
    if (presetDirectory) {
        for (const name of new Set([executable, 'xsmp_lsp'])) {
            const candidate = path.join(presetDirectory, name);
            if (fs.existsSync(candidate)) return path.resolve(candidate);
        }
    }

    // The WebAssembly server that a WASI build copies into server/. It is the
    // recommended server, so a packaged extension prefers it.
    const wasm = path.join(context.extensionPath, 'server', 'xsmp_lsp.wasm');
    if (fs.existsSync(wasm)) return wasm;

    // A native server: the packaged copy under bin/, or the one CMake built next
    // to the extension folder.
    for (const candidate of [
        path.join(context.extensionPath, 'bin', executable),
        path.join(context.extensionPath, '..', 'build', 'src', executable)
    ]) {
        if (fs.existsSync(candidate)) return path.resolve(candidate);
    }
    return undefined;
}

export function isWasmBinary(file: string): boolean {
    let descriptor: number | undefined;
    try {
        descriptor = fs.openSync(file, 'r');
        const magic = Buffer.alloc(4);
        return fs.readSync(descriptor, magic, 0, magic.length, 0) === magic.length
            && magic.equals(Buffer.from([0, 0x61, 0x73, 0x6d]));
    } catch {
        return false;
    } finally {
        if (descriptor !== undefined) fs.closeSync(descriptor);
    }
}

export function createServerOptions(serverCommand: string, context: vscode.ExtensionContext, libraries: readonly string[] = []): ServerOptions {
    const runner = path.join(context.extensionPath, 'server', 'wasi-run.mjs');
    const hostModule = path.join(context.extensionPath, 'server', 'wasi-host.mjs');
    // A WASI server loads its extension libraries before it starts a thread.
    const env = { ...process.env, XSMP_EXTENSION_LIBRARIES: libraries.join('\n') };
    if (isWasmBinary(serverCommand)) {
        if (!fs.existsSync(runner)) throw new Error(`Unable to locate the XSMP WASI runner: ${runner}`);
        if (!fs.existsSync(hostModule)) throw new Error(`Unable to locate the XSMP WASI host module: ${hostModule}`);
        return {
            command: process.execPath,
            args: ['--no-warnings', runner, '--host-module', hostModule, path.resolve(serverCommand)],
            options: {
                cwd: path.dirname(path.resolve(serverCommand)),
                // With ELECTRON_RUN_AS_NODE, the executable of VS Code runs as Node.js.
                // A WebAssembly server cannot see how many CPU cores there are:
                // unless CURSIV_WORKERS says otherwise, the runner chooses for it.
                env: { ...env, ELECTRON_RUN_AS_NODE: '1' }
            }
        };
    }
    return {
        command: serverCommand,
        args: [],
        options: { cwd: path.dirname(serverCommand), env }
    };
}

export async function activate(context: vscode.ExtensionContext): Promise<void> {
    const serverCommand = resolveServerCommand(context);
    if (!serverCommand) {
        void vscode.window.showErrorMessage(
            'Unable to locate xsmp_lsp. Build the project and/or set xsmp.server.path.'
        );
        return;
    }

    const channel = vscode.window.createOutputChannel('XSMP', { log: true });
    outputChannel = channel;
    context.subscriptions.push(channel);

    const runtime = isWasmBinary(serverCommand) ? 'wasi' : 'native';
    const contributions = discoverXsmpContributions(context, runtime, channel);
    let serverOptions: ServerOptions;
    try {
        serverOptions = createServerOptions(serverCommand, context, contributions.map(entry => entry.handlerPath));
    } catch (error) {
        void vscode.window.showErrorMessage(error instanceof Error ? error.message : String(error));
        return;
    }
    const fileWatcher = vscode.workspace.createFileSystemWatcher('**/*.{xsmpcat,xsmpasb,xsmpcfg,xsmplnk,xsmpsed,xsmpprofile,xsmptool,smpcat,smpasb,smpcfg,smplnk,smpsed}');
    const projectWatcher = vscode.workspace.createFileSystemWatcher('**/xsmp.project');
    context.subscriptions.push(fileWatcher, projectWatcher);
    const clientOptions: LanguageClientOptions = {
        // One server shares a workspace across all six XSMP languages.
        synchronize: {
            fileEvents: [fileWatcher, projectWatcher]
        },
        documentSelector: createXsmpDocumentSelector(),
        // Model documentation is workspace content, so command links supplied
        // by the language server must not execute arbitrary editor commands.
        // Embedded documentation registers its own narrowly trusted hover.
        markdown: { isTrusted: false, supportHtml: true },
        outputChannel
    };

    client = new LanguageClient(
        'xsmp',
        'XSMP',
        serverOptions,
        clientOptions
    );

    const languageClient = client;
    const getClient = (): LanguageClient => {
        if (!client) throw new Error('The XSMP language server is not running.');
        return client;
    };
    const registerContributions = () => registerDiscoveredXsmpContributions(languageClient, contributions, channel);
    try {
        await languageClient.start();
        await registerContributions();
    } catch (error) {
        client = undefined;
        await stopWhenReady(languageClient).catch(stopError => outputChannel?.appendLine(`Failed to stop the XSMP client: ${String(stopError)}`));
        outputChannel.appendLine(`Failed to activate XSMP extension: ${String(error)}`);
        outputChannel.show(true);
        throw error;
    }
    // A restarted server has lost the contributions.
    context.subscriptions.push(languageClient.onDidChangeState(event => {
        if (event.newState === State.Running) {
            void registerContributions().catch(error => outputChannel?.appendLine(`Failed to restore external XSMP contributions: ${String(error)}`));
        }
    }));
    registerWorkflows(context, getClient, outputChannel);
    // Let AI assistants use the language server through MCP. They see the same
    // documents as the editor, including unsaved changes.
    context.subscriptions.push(startMcpBridge('xsmp', 'XSMP', languageClient, outputChannel));
    registerEmbeddedDocumentation(context);
    registerSmpMirrorPreview(context, getClient);
}

// stop() refuses a client that is starting, and a crashed server restarts on its
// own. While it starts, start() returns the startup in progress.
async function stopWhenReady(languageClient: LanguageClient): Promise<void> {
    while (languageClient.state === State.Starting || languageClient.state === State.Running) {
        await languageClient.start().catch(() => undefined);
        if (languageClient.state === State.Running) {
            await languageClient.stop();
            return;
        }
    }
}

export async function deactivate(): Promise<void> {
    const running = client;
    client = undefined;
    if (running) await stopWhenReady(running);
}
