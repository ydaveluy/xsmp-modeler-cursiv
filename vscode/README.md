# XSMP Modeler (Cursiv)

Language services for XSMP catalogues, projects, assemblies, configurations, link bases and schedules, powered by the C++ Cursiv server.

The extension needs **VS Code 1.101 or later**. It contains a WebAssembly (WASI) server that runs with the Node.js of VS Code, on the desktop or on a Node-based remote extension host. VS Code for the Web is not supported. WASI is the recommended server; native servers can also be built for Linux, macOS and Windows.

Use the **Xsmp** commands to create a project or a file, generate a project or the whole workspace, import SMP XML, and open an SMP file as a read-only XSMP view. Keyword hovers link to the bundled reference documentation. Generation on save is enabled by default; generation at startup is optional. Generated C++ is formatted with clang-format when it is installed.

The extension needs a trusted workspace because it starts the server and contribution processes and generates files. In Restricted Mode, the language grammars still work, but the server and the generators do not start. With SSH, WSL or containers, install the extension on the remote host. Virtual workspaces without a local filesystem are not supported.

## Build and package

You need Node 22.13 or later, npm, CMake 3.30 or later, Ninja and wasi-sdk 34 (`cmake -P tools/fetch-wasi-sdk.cmake` installs it). From the repository root:

```sh
cmake --preset wasi-release
cmake --build --preset wasi-release --target xsmp_lsp --parallel
npm --prefix vscode ci
npm --prefix vscode run package
```

The WASI build copies the server to `vscode/server/xsmp_lsp.wasm`, next to the runner, and `npm run package` puts them in the `.vsix`. Install the package through **Extensions: Install from VSIX** or `code --install-extension <package.vsix>`.

To package a native server instead, build it with a native preset and copy the executable to `vscode/bin/`. Remove `vscode/server/xsmp_lsp.wasm` first, because the extension prefers it. The package then works on that platform only, so pass the platform to `vsce`, for example `npm --prefix vscode run package -- --target win32-x64`. On Windows, configure with `-DCMAKE_EXE_LINKER_FLAGS=-static`, so that the executable does not need the DLLs of the compiler.

## Develop with F5

Open the repository root in VS Code and install the recommended CMake Tools extension. Both launch configurations open the `example/` workspace:

- **Run XSMP Extension** builds the server with the selected CMake preset, installs the npm dependencies and compiles the extension. `CURSIV_SERVER_DIR` points to the build folder of that preset; native and WASI presets both work.
- **Run XSMP + TAS-MDK Extensions (WASI Debug)** configures `wasi-debug`, builds the WASI server and the TAS-MDK handler, and loads both extensions in one Extension Development Host. Set `WASI_SDK` if the SDK is not in a standard location. This preset uses the `build-wasi-debug` folder.

Set `xsmp.server.path` to the absolute path of a server executable or WASI module to choose the server yourself. A WebAssembly module is recognized from its content, even without a `.wasm` extension. Without this setting, the extension looks in `CURSIV_SERVER_DIR`, then for `server/xsmp_lsp.wasm`, then in `bin/` and `build/src/`. The setting wins over the F5 preset; clear it to use the preset.

Use **Developer: Reload Window** after changing the server path. When the server crashes, the language client restarts it and the extension registers the external contributions again. Contribution handlers run with the Node.js of VS Code, so no system `node` command is needed.

## Language services and contributions

The server provides the six XSMP languages with specialized completion, typed value snippets, template parameter navigation, type definitions, symbols, folding, formatting, rename, and the project and file wizards.

SDK, ESA-CDK, SMP XML, Python and AsciiDoc contributions are built in. TAS-MDK is a separate extension in `extensions/tas-mdk`: build its handler, then package and install it next to this one. Its C++ handler runs through WASI.

External extensions declare `contributes.xsmp` entries. The extension looks for them at startup and after a server restart. A handler is a library the server loads as it starts and calls directly, as described in [Create Tools and Profiles](https://github.com/ydaveluy/xsmp-modeler-cursiv/blob/main/vscode/docs/extending/tools-and-profiles.md).

Run `npm test` here for the tests of the extension.
