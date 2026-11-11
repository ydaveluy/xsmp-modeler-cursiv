// XSMP application imports for the generic Cursiv WASI runner.
import { spawnSync } from 'node:child_process';
import { readFileSync } from 'node:fs';

// The file behind a path a WASI module names. The runner shows each Windows
// drive as a folder of the root (its `preopensFor`), so `/c:/a/b` is `c:\a\b`.
export function hostPath(platform, path) {
  if (platform !== 'win32') return path;
  const drive = /^\/([A-Za-z]):(?:\/(.*))?$/.exec(path);
  return drive ? `${drive[1]}:\\${(drive[2] ?? '').replaceAll('/', '\\')}` : path;
}

// Runs the request of a module and returns its answer. Numbers are 32-bit
// little-endian.
//
// The request: the timeout in milliseconds (0 for none), the output limit, the
// number of command parts, each part as its size and bytes, and the input as its
// size and bytes.
const decodeRequest = bytes => {
  const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  let at = 0;
  const number = () => { at += 4; return view.getUint32(at - 4, true); };
  const part = () => { const size = number(); at += size; return bytes.subarray(at - size, at); };
  const timeoutMs = number(), maxOutputBytes = number();
  const command = Array.from({ length: number() }, () => Buffer.from(part()).toString());
  return { timeoutMs, maxOutputBytes, command, input: part() };
};

// The answer: the failure, the exit code (-1 for a signal), the size of the
// output, the size of the errors, the output and the errors. After a failure, the
// errors hold its message.
const encodeAnswer = (failure, exitCode, output, errors) => {
  const header = Buffer.alloc(16);
  header.writeUInt32LE(failure, 0);
  header.writeInt32LE(exitCode, 4);
  header.writeUInt32LE(output.length, 8);
  header.writeUInt32LE(errors.length, 12);
  return Buffer.concat([header, output, errors]);
};

const Failure = { none: 0, cannotStart: 1, timedOut: 2, outputTooLarge: 3 };
const noBytes = Buffer.alloc(0);

// What `dylink.0` says a library needs of the memory and of the table.
function libraryLayout(bytes) {
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
  while (at < bytes.length) {
    const id = bytes[at++];
    const end = leb() + at;
    if (id === 0) {
      const size = leb();
      const name = Buffer.from(bytes.subarray(at, at + size)).toString();
      at += size;
      while (name === 'dylink.0' && at < end) {
        const kind = bytes[at++];
        const next = leb() + at;
        if (kind === 1)
          return { memorySize: leb(), memoryAlign: 2 ** leb(), tableSize: leb() };
        at = next;
      }
    }
    at = end;
  }
  throw new Error('This file is no extension library');
}

// A thread that starts has no stack yet, so it cannot call malloc: a library
// has its thread-local blocks made when it loads.
const maxThreads = 1024;

// Extension libraries share the memory of the program, and are linked as a
// dynamic linker would: a symbol comes from the program, then from the library.
//
// A thread has its own instance of the program and of each library, and its own
// table, which no other thread can change. So a program loads its libraries
// before it starts a thread, and passes `libraries`, what that load decided, to
// each thread it starts, which links them again at the same addresses.
function createLinker(memory, inherited) {
  const libraries = inherited ?? [];
  const instances = [];
  let main, table, mainSlots, failure = '';

  const heap = () => new Uint8Array(memory().buffer);
  const text = (pointer, size) => Buffer.from(heap().slice(pointer, pointer + size)).toString();

  // The slot of a function. One already there keeps its address, as C++ asks.
  const slotOf = (record, name, fn) => {
    if (name in record.slots) return record.slots[name];
    if (!mainSlots) {
      mainSlots = new Map();
      for (let slot = 1, end = record.tableBase; slot < end; ++slot)
        if (table.get(slot)) mainSlots.set(table.get(slot), slot);
    }
    let slot = mainSlots.get(fn);
    if (slot === undefined) {
      slot = table.grow(1);
      table.set(slot, fn);
      record.tableEnd = slot + 1;
      record.appended.push(name);
    }
    record.slots[name] = slot;
    return slot;
  };

  const link = (record) => {
    const bytes = readFileSync(hostPath(process.platform, record.path));
    const module = new WebAssembly.Module(bytes);
    const first = record.memoryBase === undefined;
    if (first) {
      const layout = libraryLayout(bytes);
      record.memoryBase = main.exports.aligned_alloc(
        Math.max(layout.memoryAlign, 16), (layout.memorySize + 15) & ~15);
      heap().fill(0, record.memoryBase, record.memoryBase + layout.memorySize);
      record.tableBase = table.length;
      record.tableEnd = record.tableBase + layout.tableSize;
      Object.assign(record, { slots: {}, appended: [], symbols: {} });
    }
    if (table.length < record.tableEnd) table.grow(record.tableEnd - table.length);

    const own = new Set(WebAssembly.Module.exports(module).map((entry) => entry.name));
    const imports = {}, late = [], missing = [];
    const put = (from, name, value) => { (imports[from] ??= {})[name] = value; };
    const constant = (value) => new WebAssembly.Global({ value: 'i32', mutable: false }, value);
    let instance;
    for (const { module: from, name, kind } of WebAssembly.Module.imports(module)) {
      const found = main.exports[name];
      if (kind === 'memory') put(from, name, memory());
      else if (kind === 'table') put(from, name, table);
      else if (kind === 'tag' && found) put(from, name, found);
      // Its own function: an import binds before the instance exists, so a relay.
      else if (kind === 'function' && (found || own.has(name)))
        put(from, name, found ?? ((...args) => instance.exports[name](...args)));
      else if (name === '__memory_base') put(from, name, constant(record.memoryBase));
      else if (name === '__table_base') put(from, name, constant(record.tableBase));
      else if (name === '__stack_pointer' && found) put(from, name, found);
      else if (kind === 'global' && (from === 'GOT.mem' || from === 'GOT.func')) {
        const global = new WebAssembly.Global({ value: 'i32', mutable: true }, 0);
        put(from, name, global);
        if (found) global.value = from === 'GOT.func' ? slotOf(record, name, found) : found.value;
        else if (own.has(name)) late.push({ from, name, global });
        else missing.push(name);
      } else missing.push(name);
    }
    if (missing.length)
      throw new Error(`${record.path} needs what this program does not export: ${missing.join(', ')}`);

    instance = new WebAssembly.Instance(module, imports);
    for (const { from, name, global } of late)
      global.value = from === 'GOT.func'
        ? slotOf(record, name, instance.exports[name])
        : record.memoryBase + instance.exports[name].value;
    for (const name of first ? [] : record.appended)
      table.set(record.slots[name], main.exports[name] ?? instance.exports[name]);
    for (const [name, slot] of Object.entries(first ? {} : record.symbols))
      table.set(slot, instance.exports[name]);

    if (instance.exports.__wasm_init_tls) {
      if (first) {
        const stride = (Math.max(instance.exports.__tls_size.value, 16) + 63) & ~63;
        const counter = main.exports.aligned_alloc(64, 64 + stride * maxThreads);
        new Int32Array(memory().buffer, counter, 1)[0] = 0;
        record.tls = { counter, stride };
      }
      const taken = Atomics.add(new Int32Array(memory().buffer), record.tls.counter >> 2, 1);
      if (taken >= maxThreads) throw new Error(`Too many threads for ${record.path}`);
      instance.exports.__wasm_init_tls(record.tls.counter + 64 + taken * record.tls.stride);
    }
    if (first) {
      instance.exports.__wasm_apply_data_relocs?.();
      instance.exports.__wasm_call_ctors?.();
    }
    return instance;
  };

  return {
    threadData: () => libraries,
    instantiated(instance) {
      main = instance;
      table = instance.exports.__indirect_function_table;
      for (const record of libraries) instances.push(link(record));
    },
    imports: {
      // Each answers 0 on a failure, which `load_error` then tells.
      load_library(path, size) {
        try {
          if (!table) throw new Error('This program does not load extensions');
          const record = { path: text(path, size) };
          instances.push(link(record));
          libraries.push(record);
          return instances.length;
        } catch (error) {
          failure = String(error?.message ?? error);
          return 0;
        }
      },
      load_symbol(library, name, size) {
        const symbol = text(name, size);
        const fn = instances[library - 1].exports[symbol];
        const record = libraries[library - 1];
        if (!fn) {
          failure = `${record.path} has no function ${symbol}`;
          return 0;
        }
        const slot = record.symbols[symbol] ?? (record.tableEnd = table.grow(1) + 1) - 1;
        table.set(slot, fn);
        record.symbols[symbol] = slot;
        return slot;
      },
      load_error(output, capacity) {
        const bytes = Buffer.from(failure);
        heap().set(bytes.subarray(0, capacity), output);
        return bytes.length;
      },
    },
  };
}

export function createHost({ memory, threadData }) {
  let answer;
  const hostCall = (input, length, output, capacity) => {
    const heap = () => new Uint8Array(memory().buffer);
    if (length) {
      try {
        const request = decodeRequest(heap().slice(input, input + length));
        const [executable, ...arguments_] =
          request.command.map(part => hostPath(process.platform, part));
        const result = spawnSync(executable, arguments_, {
          input: request.input,
          timeout: request.timeoutMs, killSignal: 'SIGKILL',
          detached: process.platform !== 'win32', maxBuffer: request.maxOutputBytes,
        });
        // A process that ends without reading its input is no failure: it gets
        // EPIPE only through the pipe that stands for the input here.
        const error = result.error?.code === 'EPIPE' ? undefined : result.error;
        if (error && result.pid && process.platform !== 'win32')
          try { process.kill(-result.pid, 'SIGKILL'); } catch { /* group already exited */ }
        answer = !error ? encodeAnswer(Failure.none, result.status ?? -1, result.stdout, result.stderr)
          : encodeAnswer(error.code === 'ETIMEDOUT' ? Failure.timedOut
            : error.code === 'ENOBUFS' ? Failure.outputTooLarge : Failure.cannotStart,
            -1, noBytes, Buffer.from(error.message));
      } catch (error) {
        answer = encodeAnswer(Failure.cannotStart, -1, noBytes, Buffer.from(String(error)));
      }
    }
    if (!answer) return 0;
    const size = answer.length;
    if (capacity >= size) { heap().set(answer, output); answer = undefined; }
    return size;
  };
  const linker = createLinker(memory, threadData);
  return {
    imports: { xsmp: { host_call: hostCall, ...linker.imports } },
    instantiated: linker.instantiated,
    threadData: linker.threadData,
  };
}
