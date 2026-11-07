# XSMP TAS-MDK for Cursiv

Install this extension alongside XSMP Modeler (Cursiv). Its manifest registers
the TAS profile dynamically, including both generators, validation and project
scaffolding. The server loads the handler when it starts and calls it directly.

Both extensions require a trusted workspace and run on the workspace extension
host, including SSH, WSL and containers. Install the Modeler VSIX first, then
this VSIX, and reload the window when VS Code requests it. The bundled WASI
handler is a library of the WASI server, so both must come from the same version.
Virtual workspaces without a local filesystem are not supported.

Build and package from the repository root:

```sh
cmake --build --preset wasi-release --target xsmp-tas-mdk-handler
cd extensions/tas-mdk
npm run package
```

For development, open the repository root and select **Run XSMP + TAS-MDK Extensions (WASI Debug)**. It builds both WASI targets and loads the base extension and this contribution in one Extension Development Host.

The command-line tool includes TAS-MDK as a builtin, which a package given with
`--extension` can replace.
