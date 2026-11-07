import * as path from 'node:path';

export function isSameOrContainedPath(parentPath: string, childPath: string): boolean {
    const relative = path.relative(parentPath, childPath);
    return relative === '' || (!relative.startsWith('..') && !path.isAbsolute(relative));
}

// The error an input box shows while `name` is not a valid project or file name.
export function invalidName(subject: string, name: string): string | undefined {
    return /^[a-zA-Z][a-zA-Z0-9_.-]*$/.test(name) ? undefined : `${subject} must follow the format [a-zA-Z][a-zA-Z0-9_.-]*.`;
}

export function toXsmpIdentifier(value: string): string {
    return value.replace(/[.-]+/g, '_');
}
