# XSMP Modeler — Cursiv

Language services, generators and tools for the six XSMP languages, written in
C++20 with [Cursiv](https://ydaveluy.github.io/cursiv/). It builds for WASI,
Linux, macOS and Windows. WASI is the recommended build: one module runs on every
platform, and the VS Code packages use it.

| To change… | Edit |
|---|---|
| Syntax | `src/core/xsmp/*Grammar.hpp` |
| AST types | `src/core/xsmp/ast.hpp` |
| Validation rules | `src/core/xsmp/validation/` |
| Language services | `src/core/xsmp/*CoreModule.cpp` |
| Editor features | `src/lsp/xsmp/*LspModule.cpp` |
| CLI behavior | `src/cli.cpp` |
| Generators, tools, profiles | `src/generator/`, `src/tools/`, `src/profiles/` |
| TAS-MDK profile | `extensions/tas-mdk/` |
| Contributions | `src/contributions/`, see [Create Tools and Profiles](vscode/docs/extending/tools-and-profiles.md) |

Each of the six languages has its own files. `Deployment*` classes hold what the
four deployment languages share.

## Build

You need CMake 3.30 or later, Ninja, Git, Node 22.13 or later, npm and the
compiler of your target. Cursiv is downloaded from its `main` branch; to use a
local copy, add `-DCURSIV_SOURCE_DIR=/path/to/cursiv` when configuring.

| Target | Compiler | Preset |
|---|---|---|
| WASI (recommended) | wasi-sdk 34 | `wasi-release` |
| Linux | A recent GCC or Clang | `gcc-release` or `clang-release` |
| macOS | Apple Clang, Xcode 26.4 or later | `clang-release` |
| Windows | GCC from MSYS2, in a UCRT64 shell | `gcc-release` |

MSVC is not supported.

The WASI build finds the SDK through `WASI_SDK`, `~/wasi-sdk-34.0` or
`/opt/wasi-sdk`. The first command below installs it, on every platform:

```sh
cmake -P tools/fetch-wasi-sdk.cmake
cmake --preset wasi-release
cmake --build --preset wasi-release --parallel
ctest --preset wasi-release --output-on-failure --parallel 4
npm --prefix vscode ci
npm --prefix vscode run package
npm --prefix extensions/tas-mdk run package
```

A native build uses the same CMake and CTest commands with its own preset. To
package a native server, see [the extension](vscode/README.md#build-and-package).

## Test

`tests/` follows `src/`: the tests of `src/core/xsmp/solver/Solver.cpp` are in
`tests/core/solver/Solver_test.cpp`, and those of TAS-MDK are in
`tests/extensions/tas-mdk/`. `tests/integration/` holds the tests that cross
several parts, such as the CLI and language server processes. `tests/support/`
holds the shared helpers, and `tests/fixtures/` the test data.

Generated code should rarely change, so each generator has goldens: a project
and the whole tree it must generate, in
`tests/fixtures/generator/<generator>/<case>/`. Any difference fails, and the
message names the first differing file and line. When a change is intended,
rewrite the goldens and review them in the diff:

```bash
XSMP_UPDATE_GOLDENS=1 <build>/tests/xsmp_tests --gtest_filter='*Golden*:GapPattern*'
```

The `golden-validity` test compiles the C++ goldens and checks the XML goldens
against the SMP schemas. The compilation needs the real headers, given by the
optional CMake variables `XSMP_SDK_2020_INCLUDE_DIRS`,
`XSMP_SDK_2025_INCLUDE_DIRS` and `XSMP_ESA_CDK_INCLUDE_DIR`.

Generated files name the present time and user in a copyright notice or a new
project. To pin the time, set the standard `SOURCE_DATE_EPOCH`, as the goldens do.

## Run

```sh
build/src/xsmp_cli validate example/06-mission-system -w example
build/src/xsmp_cli generate example/06-mission-system -w example
build/src/xsmp_cli new project Demo /tmp --profile xsmp-sdk --tool python --yes
```

Generated C++ is formatted with clang-format when it is installed.

In VS Code, select a CMake preset and launch **Run XSMP Extension**, or **Run XSMP
+ TAS-MDK Extensions (WASI Debug)** to load both extensions. Both open `example/`.
