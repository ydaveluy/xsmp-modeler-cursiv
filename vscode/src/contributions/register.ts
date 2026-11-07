import * as vscode from 'vscode';
import type { LanguageClient } from 'vscode-languageclient/node';
import { resolveContributionEntries } from './discovery';
import type { XsmpContributionRegistrationReport, XsmpResolvedContributionManifestEntry } from './xsmp-extension-types';
export function reportContributionRegistration(report: XsmpContributionRegistrationReport, channel: vscode.OutputChannel): void {
    for (const entry of report.registered) channel.appendLine(`Registered ${entry.kind} contribution '${entry.id}' from '${entry.extensionId}'.`);
    if (!report.failures.length) return;
    for (const failure of report.failures) {
        channel.appendLine(`${failure.extensionId} [${failure.phase}] ${failure.contributionId ?? failure.descriptorPath}: ${failure.message}`);
        channel.appendLine(`  descriptor: ${failure.descriptorPath}\n  handler: ${failure.handlerPath}`);
    }
    const details = report.failures.slice(0, 3).map(failure => `${failure.extensionId} (${failure.phase}): ${failure.message}`).join('\n');
    void vscode.window.showWarningMessage(`Some XSMP contributions failed to initialize.\n${details}${report.failures.length > 3 ? `\n…and ${report.failures.length - 3} more.` : ''}`, 'Show Details').then(selection => { if (selection === 'Show Details') channel.show(true); });
}
// What the other extensions contribute, for this kind of server. It stays the same
// for a window: an extension that comes later needs a reload.
export function discoverXsmpContributions(context: vscode.ExtensionContext, runtime: 'wasi' | 'native', channel: vscode.OutputChannel): XsmpResolvedContributionManifestEntry[] {
    const entries: XsmpResolvedContributionManifestEntry[] = [];
    for (const extension of vscode.extensions.all) {
        if (extension.id !== context.extension.id) entries.push(...resolveContributionEntries(extension.id, extension.extensionPath, extension.packageJSON, runtime, message => channel.appendLine(message)));
    }
    return entries;
}
export async function registerDiscoveredXsmpContributions(client: LanguageClient, entries: readonly XsmpResolvedContributionManifestEntry[], channel: vscode.OutputChannel): Promise<void> {
    if (entries.length) {
        channel.appendLine(`Registering ${entries.length} external XSMP contribution(s).`);
        reportContributionRegistration(await client.sendRequest<XsmpContributionRegistrationReport>('xsmp/registerContributions', entries), channel);
    }
}
