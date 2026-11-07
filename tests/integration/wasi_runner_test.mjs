import assert from 'node:assert/strict';
import { existsSync, mkdtempSync, rmSync, symlinkSync, writeFileSync } from 'node:fs';
import { tmpdir, homedir } from 'node:os';
import { join } from 'node:path';
import { spawnSync } from 'node:child_process';
import { fileURLToPath, pathToFileURL } from 'node:url';
import test from 'node:test';


const runner = process.env.CURSIV_TEST_WASI_RUNNER ?? process.env.XSMP_TEST_WASI_RUNNER
  ?? [fileURLToPath(new URL('../../build/_deps/cursiv-src/tools/wasi-run.mjs', import.meta.url))].find(existsSync);
if (!runner) throw new Error('Set CURSIV_TEST_WASI_RUNNER to the Cursiv tools/wasi-run.mjs path');
const hostModule = process.env.XSMP_TEST_WASI_HOST_MODULE
  ?? fileURLToPath(new URL('../../tools/wasi-host.mjs', import.meta.url));
const runnerArguments = [runner, '--host-module', hostModule];
const { hostPath } = await import(pathToFileURL(hostModule));
const argument = process.argv.indexOf('--sdk');
const sdk = argument >= 0 ? process.argv[argument + 1]
  : [process.env.WASI_SDK, join(homedir(), 'wasi-sdk-34.0'), '/opt/wasi-sdk']
      .find(path => path && existsSync(join(path, 'bin/clang')));

// The request and the answer of a host call, as src/core/xsmp/utils/Process.cpp
// frames them.
const number = value => { const bytes = Buffer.alloc(4); bytes.writeUInt32LE(value); return bytes; };
const text = value => { const bytes = Buffer.from(value); return Buffer.concat([number(bytes.length), bytes]); };
const encodeRequest = ({ command, input = '', timeoutMs = 0, maxOutputBytes = 1024 }) =>
  Buffer.concat([number(timeoutMs), number(maxOutputBytes), number(command.length),
    ...command.map(text), text(input)]);
const decodeAnswer = bytes => {
  const outputSize = bytes.readUInt32LE(8), errorsSize = bytes.readUInt32LE(12);
  assert.equal(bytes.length, 16 + outputSize + errorsSize);
  return { failure: bytes.readUInt32LE(0), exitCode: bytes.readInt32LE(4),
    output: bytes.toString('utf8', 16, 16 + outputSize),
    errors: bytes.toString('utf8', 16 + outputSize) };
};
const Failure = { none: 0, cannotStart: 1, timedOut: 2, outputTooLarge: 3 };

test('a path of a WASI module names its file on the host', () => {
  assert.equal(hostPath('win32', '/c:/ext/server/tas-mdk.wasm'), 'c:\\ext\\server\\tas-mdk.wasm');
  assert.equal(hostPath('win32', '/D:/a b/c.exe'), 'D:\\a b\\c.exe');
  assert.equal(hostPath('win32', '/c:'), 'c:\\');
  // What is no path of a drive stays: an option, a command, a host path.
  for (const other of ['--no-warnings', 'node', 'C:\\node\\node.exe', '/cc:/a', '/c:a'])
    assert.equal(hostPath('win32', other), other);
  assert.equal(hostPath('linux', '/c:/ext/x.wasm'), '/c:/ext/x.wasm');
});

test('SDK commands retain stdout/plugin stderr and select threads from module imports',
  { skip: !sdk && 'wasi-sdk is not installed' }, () => {
    const directory = mkdtempSync(join(tmpdir(), 'xsmp-wasi-host-'));
    try {
      const request = encodeRequest({ command: [process.execPath, '-e',
        "process.stderr.write('xsmp-plugin-stderr\\n');process.stdout.write('{\"ok\":true}')"] });
      const source = `#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#ifdef WITH_THREADS
#include <pthread.h>
#endif
__attribute__((import_module("xsmp"), import_name("host_call")))
extern uint32_t host_call(const char*, uint32_t, char*, uint32_t);
static void* call_plugin(void* unused) {
  const unsigned char request[] = {${[...request].join(',')}};
  char response[512];
  uint32_t header[4];
  uint32_t size = host_call((const char*)request, sizeof(request), response, sizeof(response));
  memcpy(header, response, sizeof(header));
  if (size > sizeof(response) || header[0] || header[1] || header[2] != 11
      || memcmp(response + 16, "{\\\"ok\\\":true}", 11)) abort();
  fwrite(response + 16 + 11, 1, header[3], stderr);
  fputs("xsmp-wasi-stderr\\n", stderr);
  return NULL;
}
int main(void) {
#ifdef WITH_THREADS
  if (strcmp(getenv("CURSIV_WORKERS"), "2")) return 30;
  pthread_t thread;
  if (pthread_create(&thread, NULL, call_plugin, NULL)) return 31;
  if (pthread_join(thread, NULL)) return 32;
#else
  if (strcmp(getenv("CURSIV_WORKERS"), "0")) return 33;
  call_plugin(NULL);
#endif
  if (__builtin_wasm_memory_grow(0, 1) == (size_t)-1) return 34;
  puts("xsmp-wasi-stdout");
  return 0;
}
`;
      const input = join(directory, 'host.c');
      writeFileSync(input, source);
      for (const mode of ['defined', 'imported', 'threads']) {
        const output = join(directory, `${mode}.wasm`);
        const threaded = mode === 'threads';
        const compiler = spawnSync(join(sdk, 'bin/clang'), [
          `--target=wasm32-wasip1${threaded ? '-threads' : ''}`, '-O0',
          ...(threaded ? ['-pthread', '-DWITH_THREADS'] : []),
          ...(mode !== 'defined' ? ['-Wl,--import-memory'] : []),
          '-Wl,--max-memory=67108864', input, '-o', output,
        ], { encoding: 'utf8', timeout: 30000 });
        assert.equal(compiler.status, 0, compiler.stderr || String(compiler.error));
        const result = spawnSync(process.execPath, [...runnerArguments, output], {
          encoding: 'utf8', timeout: 30000,
          env: { ...process.env, CURSIV_WORKERS: threaded ? '2' : '99' },
        });
        assert.equal(result.status, 0, `${mode}: ${result.stderr || result.error}`);
        assert.equal(result.stdout, 'xsmp-wasi-stdout\n', mode);
        assert.match(result.stderr, /xsmp-plugin-stderr/, mode);
        assert.match(result.stderr, /xsmp-wasi-stderr/, mode);
      }
    } finally {
      rmSync(directory, { recursive: true, force: true });
    }
  });

test('host calls enforce the time and output limits',
  { skip: !sdk && 'wasi-sdk is not installed' }, () => {
    const directory = mkdtempSync(join(tmpdir(), 'xsmp-wasi-deadline-'));
    try {
      const source = join(directory, 'bridge.c');
      const output = join(directory, 'bridge.wasm');
      writeFileSync(source, `#include <stdint.h>
#include <stdio.h>
__attribute__((import_module("xsmp"), import_name("host_call")))
extern uint32_t host_call(const char*, uint32_t, char*, uint32_t);
static char input[2097152], output[16384];
int main(void) {
  size_t size = fread(input, 1, sizeof(input), stdin);
  uint32_t result = host_call(input, size, output, sizeof(output));
  if (result > sizeof(output)) return 50;
  fwrite(output, 1, result, stdout);
}
`);
      const compiled = spawnSync(join(sdk, 'bin/clang'), [source, '-o', output],
        { encoding: 'utf8', timeout: 30000 });
      assert.equal(compiled.status, 0, compiled.stderr || String(compiled.error));
      const invoke = (program, options = {}) => {
        const result = spawnSync(process.execPath, [...runnerArguments, output], {
          timeout: 10000,
          input: encodeRequest({ command: [process.execPath, '-e', program],
            timeoutMs: 30000, ...options }),
        });
        assert.equal(result.status, 0, result.stderr.toString() || String(result.error));
        assert.doesNotMatch(result.stderr.toString(), /unrecognized flag/);
        return decodeAnswer(result.stdout);
      };
      assert.equal(invoke('setInterval(()=>{},1000)', { timeoutMs: 100 }).failure, Failure.timedOut);
      assert.equal(invoke("process.stdout.write('x'.repeat(8192))").failure, Failure.outputTooLarge);
      assert.equal(invoke('setTimeout(()=>process.stdout.write("late"),150)',
        { timeoutMs: 0 }).output, 'late');
      assert.equal(invoke('process.stdout.write("recovered")').output, 'recovered');
      assert.deepEqual(invoke('process.stdin.pipe(process.stdout)', { input: 'é\0' }),
        { failure: Failure.none, exitCode: 0, output: 'é\0', errors: '' });
      // A process that ends without reading its input keeps its exit code.
      assert.equal(invoke('process.exit(4)', { input: 'x'.repeat(1048576) }).exitCode, 4);
      assert.equal(invoke('process.kill(process.pid,"SIGKILL")').exitCode, -1);
      const missing = spawnSync(process.execPath, [...runnerArguments, output], {
        timeout: 10000, input: encodeRequest({ command: ['xsmp-no-such-program', '--version'] }),
      });
      assert.equal(decodeAnswer(missing.stdout).failure, Failure.cannotStart);
    } finally {
      rmSync(directory, { recursive: true, force: true });
    }
  });
