// Writes what the extension libraries take from a program, as options of its
// link: `extension-exports.mjs OUTPUT LIBRARY...`. A program exports only that, as
// each thread pays for each export. A name a library also defines is optional: the
// copy of the program wins when there is one, so that a type has one identity.
import { readFileSync, writeFileSync } from 'node:fs';

function namesOf(path) {
  const bytes = readFileSync(path);
  let at = 8;
  const leb = () => {
    let value = 0, shift = 0, part;
    do {
      part = bytes[at++];
      value += (part & 0x7f) * 2 ** shift;
      shift += 7;
    } while (part & 0x80);
    return value;
  };
  const name = () => {
    const size = leb();
    at += size;
    return bytes.subarray(at - size, at).toString();
  };
  const limits = () => {
    const flags = bytes[at++];
    leb();
    if (flags & 1) leb();
  };
  const imports = [], exports = new Set();
  while (at < bytes.length) {
    const id = bytes[at++];
    const end = leb() + at;
    if (id === 2)
      for (let count = leb(); count > 0; --count) {
        name();
        imports.push(name());
        const kind = bytes[at++];
        if (kind === 0) leb();
        else if (kind === 1) { ++at; limits(); }
        else if (kind === 2) limits();
        else if (kind === 3) at += 2;
        else { ++at; leb(); }
      }
    else if (id === 7)
      for (let count = leb(); count > 0; --count) {
        exports.add(name());
        ++at;
        leb();
      }
    at = end;
  }
  return { imports, exports };
}

const [output, ...libraries] = process.argv.slice(2);
const required = new Set(), optional = new Set();
for (const library of libraries) {
  const { imports, exports } = namesOf(library);
  for (const name of imports)
    if (!['memory', '__indirect_function_table', '__memory_base', '__table_base',
          '__stack_pointer'].includes(name))
      (exports.has(name) ? optional : required).add(name);
}
const options = [...optional].filter((name) => !required.has(name))
  .map((name) => `-Wl,--export-if-defined=${name}`);
options.push(...[...required].map((name) => `-Wl,--export=${name}`));
writeFileSync(output, options.sort().join('\n') + '\n');
