import * as path from 'node:path';
import * as vscode from 'vscode';
import type { LanguageClient } from 'vscode-languageclient/node';
// SMP XML extensions and the XSMP extensions that mirror them.
export const smpToXsmpExtensions: Readonly<Record<string, string>> = { '.smpcat': '.xsmpcat', '.smpasb': '.xsmpasb', '.smpcfg': '.xsmpcfg', '.smplnk': '.xsmplnk', '.smpsed': '.xsmpsed' };
// The virtual XSMP document of an SMP XML file, which keeps its name; undefined
// for any other document.
export function smpMirrorUri(source: vscode.Uri): vscode.Uri | undefined {
    if (source.scheme !== 'file' || !smpToXsmpExtensions[path.extname(source.path)]) return undefined;
    return source.with({ scheme: 'xsmp-smp' });
}
// The XSMP file that stands beside an SMP XML file, and replaces its mirror.
export function nativeXsmpUri(source: vscode.Uri): vscode.Uri {
    const extension = path.extname(source.path);
    return source.with({ path: source.path.slice(0, -extension.length) + smpToXsmpExtensions[extension] });
}
// The XSMP language of a mirror, from the kind of its XML file; undefined for
// any other document, and for the mirror URIs of earlier versions, which ended
// with the XSMP extension.
export function smpMirrorLanguageId(uri: vscode.Uri): string | undefined {
    return uri.scheme === 'xsmp-smp' ? smpToXsmpExtensions[path.extname(uri.path)]?.slice(1) : undefined;
}
// The mirror that a mirror URI of an earlier version stands for, which ended
// with the XSMP extension; undefined for any other URI.
export function legacySmpMirrorUri(uri: vscode.Uri): vscode.Uri | undefined {
    if (uri.scheme !== 'xsmp-smp') return undefined;
    const extension = path.extname(uri.path);
    const xml = Object.entries(smpToXsmpExtensions).find(([, native]) => native === extension)?.[0];
    return xml ? uri.with({ path: uri.path.slice(0, -extension.length) + xml }) : undefined;
}
// What to open for an SMP XML file: the XSMP file beside it, else its virtual
// document; undefined when the server has no XSMP view of it.
export async function resolveSmpMirror(source: vscode.Uri, getClient: () => LanguageClient): Promise<vscode.Uri | undefined> {
    const virtual = smpMirrorUri(source);
    if (!virtual) return undefined;
    const native = nativeXsmpUri(source);
    try {
        await vscode.workspace.fs.stat(native);
        return native;
    } catch (error) {
        if (!(error instanceof vscode.FileSystemError && error.code === 'FileNotFound')) throw error;
    }
    const content = await getClient().sendRequest<{ text: string }>('workspace/textDocumentContent', { uri: virtual.toString() });
    return content?.text ? virtual : undefined;
}
