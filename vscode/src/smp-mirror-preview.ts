import * as vscode from 'vscode';
import type { LanguageClient } from 'vscode-languageclient/node';

import { legacySmpMirrorUri, resolveSmpMirror, smpMirrorLanguageId } from './smp-mirror-preview-support.js';

export const SmpMirrorPreviewViewType = 'xsmp.smpMirrorPreview';

interface SmpMirrorPreviewDocument extends vscode.CustomDocument {
    readonly uri: vscode.Uri;
}

export function registerSmpMirrorPreview(
    context: vscode.ExtensionContext,
    getClient: () => LanguageClient,
): void {
    context.subscriptions.push(
        vscode.window.registerCustomEditorProvider(
            SmpMirrorPreviewViewType,
            new SmpMirrorPreviewEditorProvider(getClient),
            {
                webviewOptions: {
                    retainContextWhenHidden: false,
                },
            },
        ),
        // A mirror keeps the name of its XML file: its language comes from the
        // kind of that file, not from its extension.
        vscode.workspace.onDidOpenTextDocument(document => void setMirrorLanguage(document)),
        // An editor restored from an earlier version names the mirror by its
        // XSMP extension: the mirror of the XML file takes its place.
        vscode.window.tabGroups.onDidChangeTabs(event => {
            for (const tab of event.opened) void reopenLegacyMirror(tab);
        }),
    );
    for (const document of vscode.workspace.textDocuments) void setMirrorLanguage(document);
    for (const group of vscode.window.tabGroups.all) {
        for (const tab of group.tabs) void reopenLegacyMirror(tab);
    }
}

async function setMirrorLanguage(document: vscode.TextDocument): Promise<void> {
    const languageId = smpMirrorLanguageId(document.uri);
    if (!languageId || document.isClosed || document.languageId === languageId) return;
    try {
        // The change opens the document again, with the language it now has.
        await vscode.languages.setTextDocumentLanguage(document, languageId);
    } catch (error) {
        console.error(`Failed to select the XSMP language for ${document.uri.toString()}`, error);
    }
}

async function reopenLegacyMirror(tab: vscode.Tab): Promise<void> {
    if (!(tab.input instanceof vscode.TabInputText)) return;
    const legacy = tab.input.uri;
    const mirror = legacySmpMirrorUri(legacy);
    if (!mirror) return;
    const viewColumn = tab.group.viewColumn;
    try {
        await vscode.window.showTextDocument(await vscode.workspace.openTextDocument(mirror), {
            viewColumn,
            preserveFocus: true,
            preview: tab.isPreview,
        });
        // The group holds the new editor before the old one closes.
        const old = vscode.window.tabGroups.all.find(group => group.viewColumn === viewColumn)?.tabs
            .find(candidate => candidate.input instanceof vscode.TabInputText && candidate.input.uri.toString() === legacy.toString());
        if (old) await vscode.window.tabGroups.close(old, true);
    } catch (error) {
        console.error(`Failed to reopen the SMP mirror ${legacy.toString()}`, error);
    }
}

class SmpMirrorPreviewEditorProvider implements vscode.CustomReadonlyEditorProvider<SmpMirrorPreviewDocument> {
    protected readonly getClient: () => LanguageClient;

    constructor(getClient: () => LanguageClient) {
        this.getClient = getClient;
    }

    async openCustomDocument(uri: vscode.Uri): Promise<SmpMirrorPreviewDocument> {
        return {
            uri,
            dispose() { },
        };
    }

    async resolveCustomEditor(
        document: SmpMirrorPreviewDocument,
        webviewPanel: vscode.WebviewPanel,
    ): Promise<void> {
        webviewPanel.webview.html = renderLoadingHtml(document.uri);

        const targetUri = await resolveSmpMirror(document.uri, this.getClient) ?? document.uri;
        const textDocument = await vscode.workspace.openTextDocument(targetUri);
        await vscode.window.showTextDocument(textDocument, {
            viewColumn: webviewPanel.viewColumn,
            preview: true,
            preserveFocus: true,
        });

        const customEditorTab = findCustomEditorTab(document.uri);
        if (customEditorTab) {
            await vscode.window.tabGroups.close(customEditorTab, true);
        }
    }
}

function findCustomEditorTab(uri: vscode.Uri): vscode.Tab | undefined {
    for (const group of vscode.window.tabGroups.all) {
        for (const tab of group.tabs) {
            if (!(tab.input instanceof vscode.TabInputCustom)) {
                continue;
            }
            if (tab.input.viewType !== SmpMirrorPreviewViewType) {
                continue;
            }
            if (tab.input.uri.toString() === uri.toString()) {
                return tab;
            }
        }
    }
    return undefined;
}

function renderLoadingHtml(uri: vscode.Uri): string {
    return `<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            margin: 0;
            padding: 1rem;
            color: var(--vscode-editor-foreground);
            background: var(--vscode-editor-background);
            font-family: var(--vscode-font-family);
        }
    </style>
</head>
<body>
    Opening XSMP for ${escapeHtml(uri.fsPath)}...
</body>
</html>`;
}

function escapeHtml(value: string): string {
    return value
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#39;');
}
