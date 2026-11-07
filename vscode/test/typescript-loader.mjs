import fs from 'node:fs';
import path from 'node:path';
import vm from 'node:vm';
import { createRequire } from 'node:module';
const require = createRequire(new URL('../package.json', import.meta.url));
// Without its worker thread, esbuild leaves nothing running after a transform.
process.env.ESBUILD_WORKER_THREADS = '0';
const esbuild = require('esbuild');
export function createTypeScriptLoader(mocks = {}) {
    const modules = new Map();
    function load(file) {
        file = path.resolve(file);
        if (modules.has(file)) return modules.get(file).exports;
        const module = { exports: {} }; modules.set(file, module);
        const text = esbuild.transformSync(fs.readFileSync(file, 'utf8'), { loader: 'ts', format: 'cjs', target: 'es2020' }).code;
        const localRequire = id => {
            if (Object.hasOwn(mocks, id)) return mocks[id];
            if (id.startsWith('.')) {
                const resolved = path.resolve(path.dirname(file), id).replace(/\.js$/, '.ts');
                return load(resolved.endsWith('.ts') ? resolved : resolved + '.ts');
            }
            return require(id);
        };
        vm.runInNewContext(text, { require: localRequire, module, exports: module.exports, process, Buffer, console, Error, Date, Map, Set, RegExp, Symbol, setTimeout, clearTimeout, URL, __filename: file, __dirname: path.dirname(file) }, { filename: file });
        return module.exports;
    }
    return load;
}
// The absolute path of a file under src/.
export const src = file => new URL('../src/' + file, import.meta.url).pathname;
