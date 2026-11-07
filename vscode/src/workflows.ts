import * as path from 'node:path';
import { createProjectWizard } from './wizard/wizard';
import { createXsmpStarterFileWizard } from './wizard/file-wizard';
import * as vscode from 'vscode';
import { legacySmpMirrorUri, nativeXsmpUri, resolveSmpMirror, smpMirrorLanguageId, smpMirrorUri } from './smp-mirror-preview-support';
import { xsmpLanguageIds } from './xsmp-language-support';
import type { LanguageClient } from 'vscode-languageclient/node';

const starterFiles = [
    { command: 'xsmp.newCatalogue', kind: 'catalogue' },
    { command: 'xsmp.newAssembly', kind: 'assembly' },
    { command: 'xsmp.newConfiguration', kind: 'configuration' },
    { command: 'xsmp.newLinkBase', kind: 'link-base' },
    { command: 'xsmp.newSchedule', kind: 'schedule' }
] as const;
// The LSP error code of a request that its client cancelled.
const requestCancelled = -32800;
type Report = { generatedProjects?: string[]; skippedProjects?: { projectName: string; errorCount: number }[]; failures?: { message: string }[] };

export function registerWorkflows(context: vscode.ExtensionContext, getClient: () => LanguageClient, channel?: vscode.OutputChannel): void {
    const execute = <T>(method: string, param?: unknown) => getClient().sendRequest<T>(method, param);
    const safely = async (work: () => Promise<unknown>): Promise<void> => {
        try { await work(); } catch (error) { void vscode.window.showErrorMessage(error instanceof Error ? error.message : String(error)); }
    };
    const command = (id: string, work: (uri?: vscode.Uri) => Promise<unknown>) => vscode.commands.registerCommand(id, (uri?: vscode.Uri) => safely(() => work(uri)));
    const active = (uri?: vscode.Uri) => uri ?? vscode.window.activeTextEditor?.document.uri;
    async function generate(uri?: vscode.Uri, all = false, automatic = false, successPrefix?: string): Promise<void> {
        const target = active(uri);
        if (!all && !target) throw new Error('Open or select a project document before running XSMP generation.');
        // A generation runs under a notification that can cancel it, and then
        // reports nothing.
        const report = await vscode.window.withProgress({ location: vscode.ProgressLocation.Notification, title: 'Generating XSMP sources', cancellable: true }, async (_progress, token) => {
            try {
                return await getClient().sendRequest<Report>(all ? 'xsmp/generateAllProjects' : 'xsmp/generateProject', all ? undefined : target!.toString(), token);
            } catch (error) {
                if (error instanceof vscode.CancellationError || (error as { code?: unknown } | undefined)?.code === requestCancelled) return undefined;
                throw error;
            }
        });
        if (!report) return;
        if (report.failures?.length) throw new Error(report.failures.map(f => f.message).join('\n'));
        if (automatic) {
            if (report.skippedProjects?.length) {
                const details = report.skippedProjects.map(project => `${project.projectName} (${project.errorCount} error${project.errorCount === 1 ? '' : 's'})`).join(', ');
                void vscode.window.showWarningMessage(`XSMP generation on save skipped ${report.skippedProjects.length} project(s): ${details}`);
            } else if (report.generatedProjects?.[0]) channel?.appendLine(`Generated project '${report.generatedProjects[0]}' after saving ${target!.fsPath}.`);
        } else reportProjectGeneration(report, successPrefix ?? (all ? 'XSMP workspace generation completed.' : 'XSMP project generation completed.'));
    }
    async function importSmp(uri?: vscode.Uri): Promise<void> {
        const input = resolveImportSmpTarget(active(uri));
        if (!input) {
            void vscode.window.showErrorMessage('Open or select an SMP XML file (.smpcat, .smpcfg, .smplnk, .smpasb, or .smpsed) before importing.');
            return;
        }
        const output = nativeXsmpUri(input);
        let force = false;
        try { await vscode.workspace.fs.stat(output); force = await vscode.window.showWarningMessage(`'${path.basename(output.fsPath)}' already exists.`, { modal: true }, 'Overwrite') === 'Overwrite'; if (!force) return; }
        catch (error) { if (!(error instanceof vscode.FileSystemError && error.code === 'FileNotFound')) throw error; }
        const result = await execute<{ outputPath: string; warnings: string[] }>('xsmp/importSmpFile', { uri: input.toString(), outputUri: output.toString(), force });
        for (const warning of result.warnings) channel?.appendLine(`SMP import warning: ${warning}`);
        await vscode.window.showTextDocument(await vscode.workspace.openTextDocument(vscode.Uri.file(result.outputPath)), { preview: false });
        if (result.warnings.length) void vscode.window.showWarningMessage(`Imported SMP file with ${result.warnings.length} warning(s). See "XSMP" for details.`);
        else void vscode.window.showInformationMessage(`Imported SMP file to '${path.basename(result.outputPath)}'.`);
    }
    async function mirror(uri?: vscode.Uri): Promise<void> {
        const source = active(uri);
        if (!source || !smpMirrorUri(source)) throw new Error('Open or select an SMP XML file (.smpcat, .smpcfg, .smplnk, .smpasb, or .smpsed) before opening XSMP.');
        const target = await resolveSmpMirror(source, getClient);
        if (!target) { void vscode.window.showInformationMessage(`No XSMP view is currently available for '${source.fsPath}'.`); return; }
        await vscode.window.showTextDocument(await vscode.workspace.openTextDocument(target), { preview: false });
    }
    context.subscriptions.push(command('xsmp.wizard', () => createProjectWizard(getClient())), command('xsmp.newFile', uri => createXsmpStarterFileWizard(undefined, uri)),
        ...starterFiles.map(starter => command(starter.command, uri => createXsmpStarterFileWizard(starter.kind, uri))), command('xsmp.generate', uri => generate(uri)), command('xsmp.generateAll', () => generate(undefined, true)),
        command('xsmp.importSmp', importSmp), command('xsmp.openSmpMirror', mirror), command('xsmp.openSmpXmlSource', async uri => {
            const source = resolveImportSmpTarget(active(uri));
            if (!source) { void vscode.window.showErrorMessage('Open an SMP XML mirror or source file before reopening the XML source.'); return; }
            await vscode.window.showTextDocument(await vscode.workspace.openTextDocument(source), { preview: false });
        }), vscode.workspace.onDidSaveTextDocument(async document => {
            if (!shouldGenerateOnSave(document)) return;
            try { await generate(document.uri, false, true); }
            catch (error) { channel?.appendLine(`Automatic XSMP generation on save failed for ${document.uri.toString()}: ${String(error)}`); void vscode.window.showErrorMessage('XSMP generation on save failed. See "XSMP" for details.'); }
        }));
    if (vscode.workspace.workspaceFolders?.length && vscode.workspace.getConfiguration('xsmp').get<boolean>('generateAllOnStartup', false)) void safely(() => generate(undefined, true, false, 'XSMP startup generation completed.'));
}

export function shouldGenerateOnSave(document: vscode.TextDocument): boolean {
    return !document.isUntitled && document.uri.scheme === 'file'
        && (xsmpLanguageIds as readonly string[]).includes(document.languageId)
        && vscode.window.activeTextEditor?.document.uri.toString() === document.uri.toString()
        && vscode.workspace.getConfiguration('xsmp').get<boolean>('generateOnSave', true);
}

export function resolveImportSmpTarget(candidate: vscode.Uri | undefined): vscode.Uri | undefined {
    if (!candidate) return undefined;
    if (smpMirrorUri(candidate)) return candidate;
    if (candidate.scheme === 'xsmp-smp') {
        const mirror = smpMirrorLanguageId(candidate) ? candidate : legacySmpMirrorUri(candidate);
        if (mirror) return mirror.with({ scheme: 'file', query: '', fragment: '' });
    }
    return undefined;
}

export function reportProjectGeneration(report: Report, successPrefix: string): void {
    const generated = report.generatedProjects ?? [], skipped = report.skippedProjects ?? [];
    if (!skipped.length) {
        const message = !generated.length ? `${successPrefix} No project was generated.`
            : generated.length === 1 ? `${successPrefix} Generated project '${generated[0]}'.`
            : `${successPrefix} Generated ${generated.length} project(s).`;
        void vscode.window.showInformationMessage(message);
    } else {
        const details = skipped.map(project => `${project.projectName} (${project.errorCount} error${project.errorCount === 1 ? '' : 's'})`).join(', ');
        void vscode.window.showWarningMessage(`${generated.length ? `Generated ${generated.length} project(s). ` : ''}Skipped ${skipped.length} project(s): ${details}`);
    }
}
