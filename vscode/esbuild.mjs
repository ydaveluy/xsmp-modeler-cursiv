import { rmSync, cpSync, existsSync, readFileSync, readdirSync, statSync, writeFileSync } from 'node:fs';
import path from 'node:path';
import esbuild from 'esbuild';

const production = process.argv.includes('--production');
const watch = process.argv.includes('--watch');

// Start from a clean output folder, so that no stale file reaches the .vsix.
if (!watch) {
  rmSync('out', { recursive: true, force: true });
}

// Bundle the extension into a single CommonJS file. Only `vscode` is external
// (the editor injects it at runtime); everything else — including
// vscode-languageclient — is bundled in, so the packaged .vsix is self-contained.
const context = await esbuild.context({
  entryPoints: { extension: 'src/extension.ts' },
  bundle: true,
  format: 'cjs',
  platform: 'node',
  target: 'node20',
  outdir: 'out',
  external: ['vscode'],
  sourcemap: !production,
  minify: production,
  logLevel: 'info',
  metafile: true,
});

if (watch) {
  await context.watch();
} else {
  const result = await context.rebuild();
  // Collect the license notices of the dependencies bundled into the extension.
  const packages = new Map();
  for (const input of Object.keys(result.metafile.inputs)) {
    if (!input.split(/[\\/]/).includes('node_modules')) continue;
    let directory = path.dirname(path.resolve(input));
    while (!existsSync(path.join(directory, 'package.json'))) {
      const parent = path.dirname(directory);
      if (parent === directory) throw new Error(`Cannot identify bundled dependency: ${input}`);
      directory = parent;
    }
    const metadata = JSON.parse(readFileSync(path.join(directory, 'package.json'), 'utf8'));
    // Some packages have inner package.json files defining only module format.
    while (!metadata.name && directory.includes(`${path.sep}node_modules${path.sep}`)) {
      directory = path.dirname(directory);
      if (existsSync(path.join(directory, 'package.json'))) Object.assign(metadata, JSON.parse(readFileSync(path.join(directory, 'package.json'), 'utf8')));
    }
    if (packages.has(`${metadata.name}@${metadata.version}`)) continue;
    const notices = readdirSync(directory).filter(file => /^(licen[sc]e|copying|notice)(\.[^.]+)?$/i.test(file) && statSync(path.join(directory, file)).isFile());
    if (!notices.length) throw new Error(`Bundled dependency has no license notice: ${metadata.name}`);
    packages.set(`${metadata.name}@${metadata.version}`, notices.map(file => readFileSync(path.join(directory, file), 'utf8')).join('\n'));
  }
  writeFileSync('out/THIRD-PARTY-LICENSES.txt', [...packages].sort(([a], [b]) => a.localeCompare(b)).map(([name, license]) => `${name}\n${'='.repeat(name.length)}\n${license.trim()}\n`).join('\n'));
  // The documentation that the keyword hover opens.
  cpSync('docs', 'out/docs', { recursive: true });
  await context.dispose();
}
