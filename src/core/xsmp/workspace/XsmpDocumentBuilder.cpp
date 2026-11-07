#include <cursiv/workspace/Document.hpp>
#include <string>
#include <string_view>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/workspace/XsmpDocumentBuilder.hpp>

namespace xsmp {

XsmpDocumentBuilder::XsmpDocumentBuilder(XsmpSharedCoreServices& shared)
    : cursiv::DefaultDocumentBuilder(shared), _shared(shared) {}

std::vector<const cursiv::ValidationCategory*>
XsmpDocumentBuilder::findMissingValidationCategories(
    const cursiv::Document& document, const cursiv::CoreServices& language,
    std::span<const cursiv::ValidationCategory* const>) const {
  std::vector<const cursiv::ValidationCategory*> categories{&cursiv::fast,
                                                            &cursiv::built_in};
  const auto* project =
      _shared.contributionCategories.empty() || !_shared.projectManager
          ? nullptr
          : _shared.projectManager->getProject(document);
  if (project)
    for (const auto* element : project->elements) {
      std::string_view selected;
      if (const auto* profile = cursiv::ast_cast<ProfileReference>(element))
        selected = profile->profile.refText;
      else if (const auto* tool = cursiv::ast_cast<ToolReference>(element))
        selected = tool->tool.refText;
      else
        continue;
      if (const auto alias =
              _shared.contributionAliases.find(std::string(selected));
          alias != _shared.contributionAliases.end())
        selected = alias->second;
      if (const auto category = _shared.contributionCategories.find(selected);
          category != _shared.contributionCategories.end())
        categories.push_back(&category->second);
    }
  return cursiv::DefaultDocumentBuilder::findMissingValidationCategories(
      document, language, categories);
}

}  // namespace xsmp
