# Recommended Workflow

XSMP Modeler supports several ways of working, but the most productive user workflow is usually the following.

## 1. Start with a project

Create or open a project with:

- the VS Code wizard
- or an existing `xsmp.project`

Declare the project sources, profile and tools first. This gives the editor enough context to validate the rest of the workspace correctly.

## 2. Define your types in catalogues

Start modeling in `.xsmpcat` files:

- value types
- enumerations
- interfaces
- models
- services

Catalogues are the foundation for everything else.

If you already have SMP XML sources, you can also place `.smpcat`, `.smpcfg`, `.smplnk`, `.smpasb`, or `.smpsed` files directly in a project source folder. XSMP Modeler imports them automatically for editing, validation, and generation workflows. If a real XSMP source file and an SMP source homologue exist side by side, the XSMP file wins.

## 3. Add runtime documents as needed

Use the other XSMP languages when the project grows:

- `.xsmpcfg` for reusable configuration data
- `.xsmpasb` for instantiated systems
- `.xsmplnk` for reusable link wiring
- `.xsmpsed` for schedules and activities

Not every project needs all of them.

## 4. Reuse examples

The `example/` folder of the repository is a good place to study realistic structures:

- `01-foundation`
- `02-avionics`
- `04-orbital-segment`
- `06-mission-system`

Those examples show how catalogues, configurations, assemblies, links and schedules fit together.

## 5. Validate early

Use validation continuously in the editor and fix issues as soon as they appear.

## 6. Generate outputs

In VS Code, you can either rely on automatic generation on save when `xsmp.generateOnSave` is enabled or run commands explicitly:

- `Xsmp: Generate Project`
- `Xsmp: Generate All Projects`

Generation uses the active profile and tools of the target project.

Each generation compares its outputs with the files on disk. Unchanged files
keep their timestamps; missing files are created again and generated files
edited by hand are restored from the model. After a successful generation,
unexpected files are removed, but only inside folders that belong to a
generator, such as `src-gen` or `include-gen`. Keep handwritten code in
generation-gap source folders or other folders of your own, not in generated
folders.

In folders shared with your own files, such as the Python folders, only the
generated files listed in `.xsmp/generated-files.json` are removed once they are
no longer generated. Files created only once are kept and never written again
while they exist. Keep the `.xsmp/` folder out of version control. If the list
cannot be read, no file is removed from these folders.

Editing a generated C++ file does not start a generation: the next generation,
on an XSMP save or from a command, updates it.

Generated C++ is formatted with clang-format when it is installed. The nearest `.clang-format` decides the style. Set the environment variable `XSMP_CLANG_FORMAT` to use another tool, such as `clang-format-18`, or to `none` to turn formatting off.

## 7. Keep project boundaries clear

When a project depends on another project:

- the modeling documents of the dependency become visible
- but the dependency's active tools and profile do not become active in the dependent project

That separation keeps large workspaces predictable.
