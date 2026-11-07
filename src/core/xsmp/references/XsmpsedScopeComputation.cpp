#include <cursiv/workspace/AstNodeDescriptionProvider.hpp>
#include <cursiv/workspace/Document.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/references/XsmpsedScopeComputation.hpp>
#include <xsmp/workspace/SourceUri.hpp>

namespace xsmp {

// A task is also exported under the name of its schedule.
std::vector<cursiv::AstNodeDescription>
XsmpsedScopeComputation::collectExportedSymbols(
    const cursiv::Document& document,
    const cursiv::CancellationToken& token) const {
  if (hasNativeSidecar(document.uri, services.shared())) return {};
  auto result =
      DeploymentScopeComputation::collectExportedSymbols(document, token);
  if (auto* schedule = cursiv::ast_cast<Schedule>(document.parseResult.value)) {
    for (auto* node : schedule->elements) {
      auto* task = const_cast<Task*>(cursiv::ast_cast<Task>(node));
      if (!task) continue;
      const auto name = document.keep_text(schedule->name, ".", task->name);
      if (auto d = services.astNodeDescriptionProvider->createDescription(
              *task, name, &document))
        result.push_back(*d);
    }
  }
  return result;
}

}  // namespace xsmp
