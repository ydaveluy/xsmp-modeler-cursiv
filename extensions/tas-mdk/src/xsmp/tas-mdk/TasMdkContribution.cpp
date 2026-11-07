#include <cursiv/services/ServiceRegistry.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <vector>
#include <xsmp/CoreServices.hpp>
#include <xsmp/tas-mdk/TasMdkContribution.hpp>
#include <xsmp/tas-mdk/TasMdkGenerator.hpp>
#include <xsmp/tas-mdk/TasMdkPythonGenerator.hpp>
#include <xsmp/tas-mdk/TasMdkValidator.hpp>

namespace xsmp {

void describeTasMdk(Contribution& contribution,
                    XsmpSharedCoreServices& shared) {
  auto& core =
      const_cast<XsmpcatCoreServices&>(dynamic_cast<const XsmpcatCoreServices&>(
          shared.serviceRegistry->getServices({}, "xsmpcat")));
  contribution.label = "TAS MDK";
  contribution.description =
      "Prepare a project targeting the TAS MDK environment.";
  contribution.generators = {
      [&core](const cursiv::AstNode& node,
              const std::filesystem::path& output) {
        if (const auto* catalogue = cursiv::ast_cast<Catalogue>(&node))
          TasMdkGenerator(core).generate(*catalogue, output);
      },
      [&core](const cursiv::AstNode& node,
              const std::filesystem::path& output) {
        if (const auto* catalogue = cursiv::ast_cast<Catalogue>(&node))
          TasMdkPythonGenerator(core).generate(*catalogue, output);
      }};
  contribution.generatedFolders = {"src-gen", "include-gen"};
  contribution.trackedFolders = {
      {.generator = "python", .output = "generated", .folder = "helpers"}};
  contribution.validations.emplace_back(
      [](XsmpSharedCoreServices& state,
         const cursiv::ValidationCategory& category) {
        auto& services = const_cast<XsmpcatCoreServices&>(
            dynamic_cast<const XsmpcatCoreServices&>(
                state.serviceRegistry->getServices({}, "xsmpcat")));
        registerTasMdkValidationChecks(services, category);
      });
  contribution.scaffolder = [](ScaffoldContext& context) {
    scaffoldTasMdkProject(context.projectDir, context.projectName);
    context.addDependency("TasMdk");
  };
}

void registerTasMdk(XsmpSharedCoreServices& shared) {
  constexpr auto alias = "org.eclipse.xsmp.profile.tas-mdk";
  Contribution contribution;
  contribution.id = "tas-mdk";
  contribution.kind = "profile";
  contribution.deprecatedAliases = {alias};
  describeTasMdk(contribution, shared);
  for (const auto& validation : contribution.validations)
    validation(shared, contributionCategory(shared, contribution.id));
  registry(shared).add(std::move(contribution));
  shared.contributionAliases[alias] = "tas-mdk";
  shared.contributionDeprecatedAliases.insert(alias);

  // The descriptor, which the `profile 'tas-mdk'` of a project resolves to.
  std::vector<cursiv::Document*> documents{
      shared.documents
          ->createDocument("xsmp:/contributions/tas-mdk.xsmpprofile",
                           "profile 'tas-mdk'\n")
          .get()};
  shared.documentBuilder->build(documents, {.validation = false});
}

}  // namespace xsmp
