import * as vscode from 'vscode';
import * as path from 'node:path';
import type { LanguageClient } from 'vscode-languageclient/node';
import { invalidName, isSameOrContainedPath } from '../utils/path-utils.js';
import type {
    XsmpContributionWizardPrompt,
    XsmpContributionKind,
    XsmpContributionSummary,
    XsmpProjectWizardPromptsRequest,
} from '../contributions/xsmp-extension-types.js';

interface ContributionQuickPickItem extends vscode.QuickPickItem {
    readonly summary: XsmpContributionSummary;
}

export async function createProjectWizard(client: LanguageClient): Promise<void> {
    const destinationFolders = await vscode.window.showOpenDialog({
        canSelectFiles: false,
        canSelectFolders: true,
        canSelectMany: false,
        title: 'Root directory containing the new project.',
        openLabel: 'Select the root directory containing the new project.',
    });

    if (!destinationFolders || destinationFolders.length === 0) {
        return;
    }

    const destinationFolder = destinationFolders[0].fsPath;
    const projectName = await vscode.window.showInputBox({
        prompt: 'Enter project name',
        validateInput: value => invalidName('Project name', value),
    });
    if (!projectName) {
        return;
    }

    const profile = await pickContribution(client, 'profile', {
        placeHolder: 'Select a profile',
    });
    const selectedTools = await pickContributions(client, 'tool', {
        placeHolder: 'Select tools to enable',
    });
    if (!selectedTools) {
        return;
    }

    const promptValues = await collectWizardPromptValues(client, profile?.summary, selectedTools.map(item => item.summary));
    if (promptValues === undefined) {
        return;
    }

    const projectFolderPath = path.join(destinationFolder, projectName);
    const projectFolderUri = vscode.Uri.file(projectFolderPath);
    try {
        await client.sendRequest('xsmp/createProject', {
            projectName,
            // A URI, because a WASI server cannot read a Windows path.
            projectDir: projectFolderUri.toString(),
            selectedProfileId: profile?.summary.id,
            selectedToolIds: selectedTools.map(item => item.summary.id),
            promptValues,
        });
    } catch (error) {
        void vscode.window.showErrorMessage(error instanceof Error ? error.message : String(error));
        return;
    }

    // Adding the first folder of a workspace restarts the extension host, so
    // the catalogue opens before that.
    const xsmpcatFilePath = path.join(projectFolderPath, 'smdl', `${projectName.replace(/\W/g, '_')}.xsmpcat`);
    const xsmpcatDocument = await vscode.workspace.openTextDocument(xsmpcatFilePath);
    await vscode.window.showTextDocument(xsmpcatDocument);

    await maybeAddProjectToWorkspace(projectFolderUri);
}

async function pickContribution(
    client: LanguageClient,
    kind: XsmpContributionKind,
    options: vscode.QuickPickOptions,
): Promise<ContributionQuickPickItem | undefined> {
    const items = await getContributionItems(client, kind);
    if (items.length === 0) {
        return undefined;
    }
    return await vscode.window.showQuickPick(items, options);
}

async function pickContributions(
    client: LanguageClient,
    kind: XsmpContributionKind,
    options: vscode.QuickPickOptions,
): Promise<readonly ContributionQuickPickItem[] | undefined> {
    const items = await getContributionItems(client, kind);
    if (items.length === 0) {
        return [];
    }
    return await vscode.window.showQuickPick(items, {
        ...options,
        canPickMany: true,
    });
}

async function getContributionItems(
    client: LanguageClient,
    kind: XsmpContributionKind,
): Promise<ContributionQuickPickItem[]> {
    const summaries = await client.sendRequest<XsmpContributionSummary[]>('xsmp/getContributionSummaries', kind);
    return summaries
        .slice()
        // A single-choice quick pick ignores `picked`: Enter takes the first item.
        .sort((left, right) => Number(right.defaultSelected) - Number(left.defaultSelected) || left.label.localeCompare(right.label))
        .map(summary => ({
            label: summary.label,
            description: summary.id === summary.label ? undefined : summary.id,
            detail: summary.description,
            picked: summary.defaultSelected,
            summary,
        }));
}

async function collectWizardPromptValues(
    client: LanguageClient,
    profile: XsmpContributionSummary | undefined,
    tools: readonly XsmpContributionSummary[],
): Promise<Readonly<Record<string, string | boolean>> | undefined> {
    const request: XsmpProjectWizardPromptsRequest = {
        selectedProfileId: profile?.id,
        selectedToolIds: tools.map(tool => tool.id),
    };
    const prompts = await client.sendRequest<XsmpContributionWizardPrompt[]>('xsmp/getContributionWizardPrompts', request);

    if (prompts.length === 0) {
        return {};
    }

    const values: Record<string, string | boolean> = {};
    for (const prompt of prompts) {
        const value = await promptForWizardValue(prompt);
        if (value === undefined) {
            return undefined;
        }
        values[prompt.key] = value;
    }
    return values;
}

async function promptForWizardValue(prompt: XsmpContributionWizardPrompt): Promise<string | boolean | undefined> {
    switch (prompt.type) {
        case 'boolean': {
            const selected = await vscode.window.showQuickPick(
                [
                    {
                        label: 'Yes',
                        value: true,
                        description: prompt.description,
                    },
                    {
                        label: 'No',
                        value: false,
                    },
                ],
                {
                    placeHolder: `${prompt.contributionId}: ${prompt.label}`,
                },
            );
            return selected?.value;
        }
        case 'choice': {
            const items = (prompt.choices ?? []).map(choice => ({
                label: choice.label ?? choice.value,
                description: choice.description,
                value: choice.value,
            }));
            if (items.length === 0) {
                return prompt.defaultValue;
            }
            const selected = await vscode.window.showQuickPick(items, {
                placeHolder: `${prompt.contributionId}: ${prompt.label}`,
            });
            return selected?.value;
        }
        case 'string':
        default:
            return await vscode.window.showInputBox({
                prompt: `${prompt.contributionId}: ${prompt.label}`,
                placeHolder: prompt.placeholder,
                value: typeof prompt.defaultValue === 'string' ? prompt.defaultValue : undefined,
            });
    }
}

async function maybeAddProjectToWorkspace(uri: vscode.Uri): Promise<void> {
    const folders = vscode.workspace.workspaceFolders ?? [];
    if (folders.some(folder => isSameOrContainedPath(folder.uri.fsPath, uri.fsPath))) {
        return;
    }
    if (await vscode.window.showQuickPick(['Yes', 'No'], { placeHolder: 'Add project to workspace?' }) === 'Yes') {
        vscode.workspace.updateWorkspaceFolders(folders.length, null, { uri });
    }
}
