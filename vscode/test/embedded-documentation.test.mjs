// The keyword hover and the command that opens the bundled documentation.
import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import { test } from 'node:test';
import { URI } from 'vscode-uri';
import { createTypeScriptLoader, src } from './typescript-loader.mjs';
import { runtime } from './vscode-mock.mjs';

const plain = value => JSON.parse(JSON.stringify(value));
const targets = JSON.parse(fs.readFileSync(new URL('./fixtures/embedded-documentation-targets.json', import.meta.url), 'utf8'));
const docs = createTypeScriptLoader()(src('embedded-documentation-links.ts'));

for (const probe of targets) {
    test(`embedded docs: ${probe.arguments.join(' / ')}`, () => {
        const actual = docs.getEmbeddedDocumentationTarget(...probe.arguments);
        if (probe.expected === null) assert.equal(actual, undefined);
        else assert.deepEqual(plain(actual), probe.expected);
    });
}

test('embedded docs open anchors and expose command links only at code positions', async t => {
    const { state, vscode, context, directory, modules } = runtime(t);
    fs.mkdirSync(path.join(directory, 'out/docs/languages'), { recursive: true }); fs.writeFileSync(path.join(directory, 'out/docs/languages/xsmpasb.md'), '# Assembly docs\n');
    modules(src('embedded-documentation.ts')).registerEmbeddedDocumentation(context);
    await state.commands.get('xsmp.openDocumentation')({ page: 'languages/xsmpasb.md', anchor: 'root-structure' });
    assert.equal(state.executed.length, 1); assert.equal(state.executed[0].command, 'markdown.showPreviewToSide'); assert.equal(state.executed[0].args[0].toString(), URI.file(path.join(directory, 'out/docs/languages/xsmpasb.md')).with({ fragment: 'root-structure' }).toString());
    const document = text => ({ languageId: 'xsmpasb', lineAt: () => ({ text }), getWordRangeAtPosition: () => new vscode.Range(new vscode.Position(0, text.indexOf('assembly')), new vscode.Position(0, text.indexOf('assembly') + 8)), getText: range => range?.start.character === 0 && range?.end.character === 4 ? text.slice(0, 4) : range ? 'assembly' : text });
    assert(state.hovers[0].provideHover(document('assembly Demo\n'), new vscode.Position(0, 1)).contents.value.includes('command:xsmp.openDocumentation?'));
    assert.equal(state.hovers[0].provideHover(document('// assembly Demo\n'), new vscode.Position(0, 4)), undefined);
});

test('embedded docs read comments and strings from the start of the document', async t => {
    const { state, vscode, context, directory, modules } = runtime(t);
    for (const page of ['xsmpcat', 'xsmpproject']) {
        fs.mkdirSync(path.join(directory, 'out/docs/languages'), { recursive: true });
        fs.writeFileSync(path.join(directory, `out/docs/languages/${page}.md`), '# Docs\n');
    }
    modules(src('embedded-documentation.ts')).registerEmbeddedDocumentation(context);
    const documentOf = (languageId, text) => {
        const lines = text.split('\n');
        const offset = position => lines.slice(0, position.line).reduce((sum, line) => sum + line.length + 1, 0) + position.character;
        return {
            languageId, lineAt: line => ({ text: lines[line] }),
            getWordRangeAtPosition: position => {
                const line = lines[position.line];
                let start = position.character, end = position.character;
                while (start > 0 && /\w/.test(line[start - 1])) start--;
                while (end < line.length && /\w/.test(line[end])) end++;
                return new vscode.Range(new vscode.Position(position.line, start), new vscode.Position(position.line, end));
            },
            getText: range => range ? text.slice(offset(range.start), offset(range.end)) : text,
        };
    };
    // On the last line, in the word that starts at `column`.
    const shown = (languageId, text, column) => {
        const lines = text.split('\n');
        const position = new vscode.Position(lines.length - 1, column(lines.at(-1)) + 2);
        return Boolean(state.hovers[0].provideHover(documentOf(languageId, text), position));
    };
    for (const [prefix, expected] of [
        ['field Smp.String8 marker = "/*"\n', true],
        ['field Smp.String8 marker = "https://example/" ', true],
        [String.raw`field Smp.String8 marker = "\"/*"` + '\n', true],
        ['field Smp.String8 marker = "\'" ', true],
        ['/* " \' // */\n', true],
        ['// "\r\n', true],
        ['/* open\n', false],
        ['// ', false],
        ['field Smp.String8 marker = "', false],
        ["field Smp.String8 marker = '", false],
    ]) assert.equal(shown('xsmpcat', `${prefix}field Smp.Int32 value = 0`, line => line.lastIndexOf('field')), expected, prefix);
    // A project string has no escape: its final backslash is text.
    assert(shown('xsmpproject', String.raw`project 'demo' source 'C:\' tool 'xsmp.tool.smp'`, line => line.indexOf('tool')));
});
