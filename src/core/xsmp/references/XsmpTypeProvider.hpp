#pragma once

#include <cursiv/syntax-tree/AstNode.hpp>
#include <cursiv/utils/WorkspaceCache.hpp>

namespace xsmp {

// Forward-declared (not included) to avoid a header cycle:
// `XsmpcatCoreServices.hpp` fully includes this header for the concrete
// `typeProvider` member type.
struct XsmpcatCoreServices;

}  // namespace xsmp

namespace xsmp {

struct Type;

}  // namespace xsmp

namespace xsmp {

/// Resolves the EXPECTED type of an expression position. Used by
/// `XsmpcatScopeProvider` to resolve a `DesignatedInitializer::field` reference
/// against the Structure type the collection literal initializes.
class XsmpTypeProvider final {
 public:
  // `services` is the concrete `XsmpcatCoreServices` container (needs
  // `services.shared` for `_typeCache`'s ctor) — only forward-declared above
  // (the same header-cycle reason as `XsmpcatValidator`), so the constructor
  // body is defined in the .cpp, where `XsmpcatCoreServices.hpp` is fully
  // included.
  explicit XsmpTypeProvider(const XsmpcatCoreServices& services);

  /// The expected type of `node` (typically an Expression), or `nullptr`
  /// when none can be determined. Memoized per-node.
  [[nodiscard]] const Type* getType(const cursiv::AstNode& node) const;

 private:
  // Reads the expected type from what contains `node`.
  [[nodiscard]] const Type* doGetType(const cursiv::AstNode& node) const;

  const XsmpcatCoreServices& services;
  cursiv::WorkspaceCache<const cursiv::AstNode*, const Type*> _typeCache;
};

}  // namespace xsmp
