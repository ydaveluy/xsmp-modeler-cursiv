import * as path from 'node:path';
import type { XsmpExtensionContributionManifestEntry, XsmpResolvedContributionManifestEntry } from './xsmp-extension-types';
// A WASI server reads each Windows drive as a folder of its root: `C:\a` is `/c:/a`.
function guestPath(file: string): string {
    return file.replace(/^([A-Za-z]):[\\/]*(.*)$/, (_, drive: string, rest: string) => `/${drive.toLowerCase()}:/${rest.replace(/\\/g, '/')}`);
}
export function resolveContributionEntries(extensionId: string, extensionRoot: string, packageJson: unknown, runtime: 'native' | 'wasi', report: (message: string) => void): XsmpResolvedContributionManifestEntry[] {
    const result: XsmpResolvedContributionManifestEntry[] = [];
    if (!isRecord(packageJson) || !isRecord(packageJson.contributes)) return result;
    const entries = packageJson.contributes.xsmp;
    if (entries === undefined) return result;
    if (!Array.isArray(entries)) {
        report(`${extensionId}: ignored contributes.xsmp: expected an array.`);
        return result;
    }
    const located = (file: string) => {
        const resolved = path.resolve(extensionRoot, file);
        return runtime === 'wasi' ? guestPath(resolved) : resolved;
    };
    for (const [index, value] of entries.entries()) {
        const issue = entryIssue(value);
        if (issue) {
            report(`${extensionId}: skipped invalid contribution at contributes.xsmp[${index}]: ${issue}.`);
            continue;
        }
        const entry = value as XsmpExtensionContributionManifestEntry;
        const native = typeof entry.nativeHandler === 'string' ? entry.nativeHandler : entry.nativeHandler?.[`${process.platform}-${process.arch}`];
        const handler = runtime === 'wasi' ? entry.wasiHandler ?? entry.handler : native ?? entry.handler;
        if (typeof handler !== 'string') { report(`${extensionId}: contribution '${entry.descriptor}' has no ${runtime} handler.`); continue; }
        result.push({ extensionId, extensionRoot: located('.'), descriptorPath: located(entry.descriptor), handlerPath: located(handler), apiVersion: entry.apiVersion, aliases: [...(entry.aliases ?? [])], deprecatedAliases: [...(entry.deprecatedAliases ?? [])], builtins: [...(entry.builtins ?? [])].map(located), wizard: entry.wizard });
    }
    return result;
}

// What makes a manifest entry unusable, or nothing.
function entryIssue(entry: unknown): string | undefined {
    if (!isRecord(entry)) return 'expected an object';
    const text = (value: unknown) => typeof value === 'string' && value.length > 0;
    if (!text(entry.descriptor)) return 'descriptor must be a non-empty string';
    for (const field of ['handler', 'wasiHandler']) {
        if (entry[field] !== undefined && !text(entry[field])) return `${field} must be a non-empty string`;
    }
    const native = entry.nativeHandler;
    if (native !== undefined && !text(native) && !(isRecord(native) && Object.values(native).every(text))) {
        return 'nativeHandler must be a non-empty string or an object of them';
    }
    if (typeof entry.apiVersion !== 'string') return 'apiVersion must be a string';
    for (const field of ['aliases', 'deprecatedAliases', 'builtins']) {
        const values = entry[field];
        if (values !== undefined && (!Array.isArray(values) || !values.every(item => typeof item === 'string'))) {
            return `${field} must be an array of strings`;
        }
    }
    const wizard = entry.wizard;
    if (wizard !== undefined) {
        if (!isRecord(wizard)) return 'wizard must be an object';
        for (const field of ['label', 'description']) {
            if (wizard[field] !== undefined && typeof wizard[field] !== 'string') return `wizard.${field} must be a string`;
        }
        if (wizard.defaultSelected !== undefined && typeof wizard.defaultSelected !== 'boolean') {
            return 'wizard.defaultSelected must be a boolean';
        }
    }
    return undefined;
}

function isRecord(value: unknown): value is Record<string, unknown> {
    return typeof value === 'object' && value !== null && !Array.isArray(value);
}
