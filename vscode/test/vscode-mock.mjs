import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { URI } from 'vscode-uri';
import { createTypeScriptLoader } from './typescript-loader.mjs';

// A small `vscode` module that records what the extension does, with a fake
// language client that answers every request with `{ text: content }`.
export function runtime(t, content = 'catalogue Demo\n') {
    const directory = fs.mkdtempSync(path.join(os.tmpdir(), 'xsmp-extension-'));
    t.after(() => fs.rmSync(directory, { recursive: true, force: true }));
    const state = { commands: new Map(), providers: new Map(), hovers: [], requests: [], opened: [], shown: [], errors: [], info: [], executed: [], closed: [], tabs: [], documents: [], languages: [], tabListeners: [], openListeners: [] };
    class Position { constructor(line, character) { this.line = line; this.character = character; } }
    class Range { constructor(start, end) { this.start = start; this.end = end; } }
    class FileSystemError extends Error { code = 'FileNotFound'; }
    class EventEmitter { event = () => ({ dispose() {} }); fire() {} dispose() {} }
    class MarkdownString { constructor(value) { this.value = value; } }
    class Hover { constructor(contents, range) { this.contents = contents; this.range = range; } }
    class TabInputCustom { constructor(uri, viewType) { this.uri = uri; this.viewType = viewType; } }
    class TabInputText { constructor(uri) { this.uri = uri; } }
    const vscode = { Uri: URI, Position, Range, FileSystemError, EventEmitter, MarkdownString, Hover, TabInputCustom, TabInputText,
        commands: { registerCommand(name, fn) { state.commands.set(name, fn); return { dispose() {} }; }, async executeCommand(command, ...args) { state.executed.push({ command, args }); } },
        languages: { registerHoverProvider(_selector, provider) { state.hovers.push(provider); return { dispose() {} }; }, async setTextDocumentLanguage(document, languageId) { state.languages.push([document.uri.toString(), languageId]); return document; } },
        workspace: { fs: { async stat() { throw new FileSystemError(); } }, onDidSaveTextDocument: () => ({ dispose() {} }), onDidOpenTextDocument(listener) { state.openListeners.push(listener); return { dispose() {} }; }, get textDocuments() { return state.documents; }, getConfiguration: () => ({ get: (_key, fallback) => fallback }), async openTextDocument(uri) { const document = { uri }; state.opened.push(document); return document; } },
        window: { async showTextDocument(document, options) { state.shown.push({ document, options }); return document; }, showErrorMessage: message => state.errors.push(message), showInformationMessage: message => state.info.push(message), registerCustomEditorProvider(name, provider, options) { state.providers.set(name, { provider, options }); return { dispose() {} }; }, get tabGroups() { return { all: state.tabs, onDidChangeTabs(listener) { state.tabListeners.push(listener); return { dispose() {} }; }, async close(tab, preserveFocus) { state.closed.push({ tab, preserveFocus }); } }; } } };
    const client = { async sendRequest(method, params) { state.requests.push([method, params]); return { text: content ?? '' }; } };
    const context = { subscriptions: [], asAbsolutePath: relative => path.join(directory, relative) };
    const modules = createTypeScriptLoader({ vscode });
    return { state, vscode, client, context, directory, modules };
}
