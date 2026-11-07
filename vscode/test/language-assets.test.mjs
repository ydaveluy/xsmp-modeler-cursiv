// The language configurations and the TextMate grammars of the six languages.
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import fs from 'node:fs';
import { test } from 'node:test';

const manifest = JSON.parse(fs.readFileSync(new URL('../package.json', import.meta.url), 'utf8'));
const read = file => fs.readFileSync(new URL('../' + file, import.meta.url));

test('language configuration: a dedicated file for each of the six languages', () => {
    assert.deepEqual(manifest.contributes.languages.map(x => x.id).sort(), ['xsmpasb', 'xsmpcat', 'xsmpcfg', 'xsmplnk', 'xsmpproject', 'xsmpsed']);
    for (const entry of manifest.contributes.languages) assert.equal(entry.configuration, `./${entry.id}-language-configuration.json`);
});

test('language configuration: delimiters, strings, templates and enter rules', () => {
    const expected = {
        xsmpproject: [[], ["'", '"', '/*'], ['"', "'"], false],
        xsmpcat: [[['{', '}'], ['[', ']'], ['(', ')']], ['[', '{', '(', "'", '"', '/*'], ['{', '[', '(', '"', "'"], false],
        xsmpasb: [[['{', '}'], ['[', ']'], ['(', ')']], ['[', '{', '(', "'", '"', '/*'], ['{', '[', '(', '"', "'", '<'], true],
        xsmpcfg: [[['{', '}'], ['[', ']']], ['[', '{', "'", '"', '/*'], ['{', '[', '"', "'"], false],
        xsmplnk: [[['{', '}'], ['[', ']']], ['[', '{', '/*'], ['{', '['], true],
        xsmpsed: [[['{', '}'], ['[', ']'], ['(', ')']], ['[', '{', '(', "'", '"', '/*'], ['{', '[', '(', '"', "'", '<'], true],
    };
    for (const [id, [brackets, closing, surrounding, templates]] of Object.entries(expected)) {
        const config = JSON.parse(read(`${id}-language-configuration.json`));
        assert.deepEqual(config.comments, { lineComment: '//', blockComment: ['/*', '*/'] });
        assert.deepEqual(config.brackets ?? [], brackets);
        assert.deepEqual(config.autoClosingPairs.map(x => x.open), closing);
        assert.deepEqual(config.surroundingPairs.map(x => x[0]), surrounding);
        for (const open of ['/*', '"', "'"]) {
            const pair = config.autoClosingPairs.find(x => x.open === open);
            assert.deepEqual(pair, closing.includes(open) ? { open, close: open === '/*' ? '*/' : open, notIn: ['string', 'comment'] } : undefined);
        }
        assert.equal(config.wordPattern.includes('\\{[_a-zA-Z]\\w*\\}\\w*'), templates);
        assert.equal(config.onEnterRules.length, 5);
    }
});

test('the six TextMate grammars and language configurations are the pinned files', () => {
    const pinned = JSON.parse(fs.readFileSync(new URL('./fixtures/textmate-assets.json', import.meta.url), 'utf8'));
    assert.deepEqual(manifest.contributes.grammars, pinned.grammars);
    assert.equal(pinned.grammars.length, 6); assert.equal(pinned.assets.length, 12);
    for (const asset of pinned.assets) {
        const bytes = read(asset.path);
        assert.equal(createHash('sha256').update(bytes).digest('hex'), asset.sha256, asset.path);
        assert.equal(bytes.length, asset.bytes, asset.path);
        JSON.parse(bytes.toString('utf8'));
    }
    for (const grammar of pinned.grammars) {
        const language = JSON.parse(read(grammar.path));
        assert.equal(language.scopeName, grammar.scopeName);
        assert.ok(language.patterns.length > 0);
    }
    assert.deepEqual(manifest.contributes.snippets ?? [], pinned.staticSnippets);
});
