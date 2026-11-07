# Install XSMP Modeler in VS Code

XSMP Modeler is distributed as a Visual Studio Code extension.

## Install the extension

You need **Visual Studio Code 1.101 or later**.

1. Download `xsmp-modeler-cursiv-<version>.vsix` from the
   [GitHub Releases page](https://github.com/ydaveluy/xsmp-modeler-cursiv/releases).
2. In VS Code, run **Extensions: Install from VSIX...** and select the file.

You can also install it from a terminal:

```bash
code --install-extension xsmp-modeler-cursiv-<version>.vsix
```

For the TAS-MDK profile, install `xsmp-tas-mdk-cursiv-<version>.vsix` of the same
release in the same way, after XSMP Modeler.

The extension runs on the desktop and on a Node-based remote host (SSH, WSL or containers). Install it on the remote host in that case. VS Code for the Web and virtual workspaces are not supported.

The extension needs a trusted workspace, because it starts the language server and can generate files.

## Open an XSMP workspace

After installation:

1. Open an existing XSMP workspace or a folder that will contain XSMP projects.
2. Open or create an `xsmp.project` file.
3. VS Code should recognize XSMP file types automatically:
   - `xsmp.project`
   - `.xsmpcat`
   - `.xsmpcfg`
   - `.xsmpasb`
   - `.xsmplnk`
   - `.xsmpsed`

## What becomes available in VS Code

Once the extension is active, you get:

- syntax highlighting
- validation diagnostics
- content assist
- hover documentation
- formatting
- outline support
- quick fixes
- built-in XSMP commands

The most important commands for daily work are:

- `Xsmp: Create a new project`
- `Xsmp: Generate Project`
- `Xsmp: Generate All Projects`

## Automatic generation options

The extension provides a setting named `xsmp.generateOnSave`.

- Default: `true`
- When enabled: VS Code automatically runs **Generate Project** for the active XSMP document when you save it

This is useful when you want generated artifacts to stay in sync while editing.

The extension also provides a setting named `xsmp.generateAllOnStartup`.

- Default: `false`
- When enabled: VS Code automatically runs **Generate All Projects** after XSMP has loaded the workspace

This is useful when you want generated artifacts to stay in sync each time the workspace opens.

## Next step

Continue with [Create a Project](create-project.md) to build your first XSMP workspace.
