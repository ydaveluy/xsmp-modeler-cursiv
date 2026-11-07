#pragma once

#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <xsmp/Contributions.hpp>

// A profile or tool from an extension is a library the server loads, so it can be
// added without rebuilding the server, and the server calls its code directly: it
// runs in this process, on this workspace, as a built-in does.
//
// Found: a `contributes.xsmp` entry in the extension's `package.json` names a
// descriptor file and a handler, which is the library: a `.wasm` for a WASI server,
// a `.so` or `.dylib` for a native one. The editor client names these libraries in
// `XSMP_EXTENSION_LIBRARIES` when it starts the server, then sends the entries with
// `xsmp/registerContributions`. The command line reads them with `--extension`.
//
// Registered: `Registry::registerManifestEntries` reads the descriptor, which gives
// the contribution its id, then calls the registration function of the library.
//
// A library shares the types of the server, so both come from the same sources:
// one built from others is refused when it loads. A native Windows server loads no
// library, as a DLL keeps its own copy of the inline variables `ast_cast` relies on:
// an extension runs under the WASI server there.

namespace xsmp {

using ExtensionRegistration = void (*)(Contribution&, XsmpSharedCoreServices&);

// What a library and the server must agree on to share their types.
inline constexpr std::string_view extensionAbi =
    XSMP_EXTENSION_ABI " " __VERSION__;

// Loads the libraries of the packages and those of `XSMP_EXTENSION_LIBRARIES`, one
// per line. A `main` calls this first: a WASI program loads a library before it
// starts a thread. A failure is kept for the registration to report.
void loadExtensionLibraries(std::span<const std::string> packages = {});

// The handlers of the `contributes.xsmp` entries of these packages. A package that
// cannot be read gives none: its registration reports that.
std::vector<std::filesystem::path> packageLibraries(
    std::span<const std::string> packages);

// The registration function of the library at `path`, which a native server loads
// now if needed. Throws when it cannot be loaded.
ExtensionRegistration extensionRegistration(const std::filesystem::path& path);

}  // namespace xsmp

// Defines, once in a library, what the server calls when it loads it.
#define XSMP_EXTENSION(Register)                                     \
  extern "C" [[gnu::visibility("default")]] const char*              \
  xsmp_extension_abi() {                                             \
    return ::xsmp::extensionAbi.data();                              \
  }                                                                  \
  extern "C" [[gnu::visibility("default")]] void xsmp_register(      \
      ::xsmp::Contribution& contribution,                            \
      ::xsmp::XsmpSharedCoreServices& shared) {                      \
    Register(contribution, shared);                                  \
  }
