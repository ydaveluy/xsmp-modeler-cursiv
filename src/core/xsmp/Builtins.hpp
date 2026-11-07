#pragma once

#include <span>
#include <string_view>

namespace cursiv {

struct SharedCoreServices;
}

namespace xsmp {

/// One embedded built-in catalogue resource (its virtual URI and source text).
struct BuiltinSource {
  std::string_view uri;
  std::string_view text;
};

/// The embedded ECSS standard catalogues (`Smp.*`, `Attributes.*`). Defined by
/// the CMake-generated `Builtins.generated.cpp`.
[[nodiscard]] std::span<const BuiltinSource> builtinSources();

/// Loads the embedded built-in catalogues into `shared`'s workspace so their
/// exported symbols are globally indexed (resolvable by every user document).
/// Call once, after the language services are registered.
void loadXsmpcatBuiltins(cursiv::SharedCoreServices& shared);

}  // namespace xsmp
