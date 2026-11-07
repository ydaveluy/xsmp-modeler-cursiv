#include <cursiv/workspace/AstNodeDescriptionProvider.hpp>
#include <cursiv/workspace/Document.hpp>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/references/XsmpprojectScopeComputation.hpp>
#include <xsmp/workspace/SourceUri.hpp>

namespace xsmp {

// A contributed profile or tool is also exported under its aliases.
std::vector<cursiv::AstNodeDescription>
XsmpprojectScopeComputation::collectExportedSymbols(
    const cursiv::Document& document,
    const cursiv::CancellationToken& token) const {
  auto exported =
      DefaultScopeComputation::collectExportedSymbols(document, token);
  const auto* shared =
      dynamic_cast<const XsmpSharedCoreServices*>(&services.shared());
  const auto* named = cursiv::ast_cast<NamedNode>(document.parseResult.value);
  if (shared && named && isBuiltinUri(document.uri)
      && (cursiv::ast_cast<Profile>(named) || cursiv::ast_cast<Tool>(named))) {
    for (const auto& [alias, canonical] : shared->contributionAliases) {
      if (canonical != named->name) continue;
      if (auto description =
              services.astNodeDescriptionProvider->createDescription(
                  *document.parseResult.value, document.keep_text(alias),
                  &document))
        exported.push_back(*description);
    }
  }
  return exported;
}

}  // namespace xsmp
