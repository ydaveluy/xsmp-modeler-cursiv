#include <exception>
#include <memory>
#include <mutex>
#include <set>
#include <xsmp/XsmpcatCoreServices.hpp>
#include <xsmp/adoc/ADocGenerator.hpp>
#include <xsmp/esa-cdk/EsaCdkGenerator.hpp>
#include <xsmp/esa-cdk/Scaffold.hpp>
#include <xsmp/python/PythonGenerator.hpp>
#include <xsmp/smp/SmpService.hpp>
#include <xsmp/utils/Parallel.hpp>
#include <xsmp/xsmp-sdk/XsmpSdkGenerator.hpp>

#include "Contributions.hpp"

namespace xsmp {

Registry makeBuiltinRegistry(const XsmpcatCoreServices& services) {
  Registry registry;
  // Runs generator `G` on a catalogue, and ignores any other document.
  const auto catalogueGenerator = [&services]<class G>() -> Generator {
    return [&services](const cursiv::AstNode& node,
                       const std::filesystem::path& directory) {
      if (const auto* catalogue = cursiv::ast_cast<const Catalogue>(&node))
        G(services).generate(*catalogue, directory);
    };
  };
  const auto scaffolding = [](auto scaffold) {
    return [scaffold](ScaffoldContext& context) {
      scaffold(context.projectDir, context.projectName);
    };
  };

  registry.add({.id = "smp",
                .label = "SMP XML",
                .generators = {[](const cursiv::AstNode& node,
                                  const std::filesystem::path& directory) {
                  if (cursiv::ast_cast<const Document>(&node)) {
                    if (const auto* document = node.getDocument())
                      (void)generateDocument(*document, directory);
                  }
                }},
                .kind = "tool",
                .description = "Generate SMP XML artifacts from XSMP models.",
                .deprecatedAliases = {"org.eclipse.xsmp.tool.smp"},
                .generatedFolders = {"smdl-gen"}});
  registry.add(
      {.id = "adoc",
       .label = "AsciiDoc",
       .generators = {[&services](const cursiv::AstNode& node,
                                  const std::filesystem::path& directory) {
         ADocGenerator().generate(node, directory);
       }},
       .kind = "tool",
       .description =
           "Generate AsciiDoc documentation outputs for the project.",
       .deprecatedAliases = {"org.eclipse.xsmp.tool.adoc"},
       .generatedFolders = {"adoc-gen"},
       .scaffolder = scaffolding(scaffoldAdocProject)});
  registry.add({.id = "python",
                .label = "Python",
                .generators =
                    {catalogueGenerator.template operator()<PythonGenerator>()},
                .kind = "tool",
                .description =
                    "Generate Python bindings and test scaffolding for "
                    "projects using the xsmp-sdk profile.",
                .deprecatedAliases = {"org.eclipse.xsmp.tool.python"},
                .trackedFolders = {{.generator = "python",
                                    .output = "python",
                                    .folder = "python"}},
                .scaffolder = scaffolding(scaffoldPythonProject)});
  registry.add({.id = "xsmp-sdk",
                .label = "XSMP-SDK",
                .generators = {catalogueGenerator
                                   .template operator()<XsmpSdkGenerator>()},
                .kind = "profile",
                .description =
                    "Recommended profile for projects using the XSMP SDK "
                    "build and test workflow.",
                .defaultSelected = true,
                .deprecatedAliases = {"org.eclipse.xsmp.profile.xsmp-sdk"},
                .generatedFolders = {"src-gen"},
                .scaffolder = scaffolding(scaffoldXsmpSdkProject)});
  registry.add({.id = "esa-cdk",
                .label = "ESA-CDK",
                .generators =
                    {catalogueGenerator.template operator()<EsaCdkGenerator>()},
                .kind = "profile",
                .description =
                    "Prepare a project targeting the ESA SMP CDK environment.",
                .deprecatedAliases = {"org.eclipse.xsmp.profile.esa-cdk"},
                .generatedFolders = {"src-gen"},
                .scaffolder = scaffolding(scaffoldEsaCdkProject)});
  return registry;
}

void Registry::generateBatch(std::span<const cursiv::AstNode* const> nodes,
                             const std::filesystem::path& directory,
                             std::span<const std::string> identifiers) const {
  for (const auto* node : nodes)
    if (!node)
      throw std::invalid_argument("Cannot generate a null syntax tree");
  std::set<std::string> seen;
  for (const auto& identifier : identifiers) {
    const auto& contribution = get(identifier);
    if (!seen.insert(contribution.id).second) continue;
    for (const auto& generator : contribution.generators)
      for (const auto* node : nodes) generator(*node, directory);
  }
}

}  // namespace xsmp
