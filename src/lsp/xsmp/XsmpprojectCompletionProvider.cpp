#include <algorithm>
#include <cctype>
#include <cursiv/parser/Parser.hpp>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <xsmp/Contributions.hpp>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/XsmpprojectCompletionProvider.hpp>
#include <xsmp/utils/XsmpDocumentation.hpp>
#include <xsmp/workspace/ProjectManager.hpp>

namespace xsmp {

namespace {

// What the file declares, from its name: `xsmp.project`, `*.xsmpprofile` or
// `*.xsmptool`.
std::string_view documentKind(std::string_view uri) {
  const auto slash = uri.find_last_of('/');
  if ((slash == uri.npos ? uri : uri.substr(slash + 1)) == "xsmp.project")
    return "project";
  if (uri.ends_with(".xsmpprofile")) return "profile";
  if (uri.ends_with(".xsmptool")) return "tool";
  return {};
}

std::string capitalized(std::string_view text) {
  std::string out(text);
  if (!out.empty())
    out[0] =
        static_cast<char>(std::toupper(static_cast<unsigned char>(out[0])));
  return out;
}

// Whether `prefix` starts `word`, whatever its case. `word` must be lower case.
bool isPrefixOfIgnoringCase(std::string_view prefix, std::string_view word) {
  return prefix.size() <= word.size()
         && std::equal(
             prefix.begin(), prefix.end(), word.begin(), [](char a, char b) {
               return std::tolower(static_cast<unsigned char>(a)) == b;
             });
}

bool writes(const cursiv::CompletionContext& context,
            const cursiv::AstTypeTag* type, std::string_view property) {
  return std::ranges::any_of(context.features, [&](const auto& next) {
    return next.type == type && next.property == property;
  });
}

// Makes the item write `name` as a project string, in place of the one the
// caret stands in. The client filters what is typed, quote included, on
// `filterText`.
void writeQuoted(cursiv::CompletionValueItem& item,
                 const cursiv::CompletionContext& context,
                 std::string_view name) {
  auto quoted = "'" + std::string(name) + "'";
  item.filterText = quoted;
  item.insertText = quoted;
  item.textEdit = ::lsp::TextEdit{.range = context.replaceRange,
                                  .newText = std::move(quoted)};
}

}  // namespace

const XsmpSharedCoreServices* XsmpprojectCompletionProvider::xsmpShared()
    const {
  return dynamic_cast<const XsmpSharedCoreServices*>(&services.shared());
}

const Registry* XsmpprojectCompletionProvider::registry() const {
  const auto* shared = xsmpShared();
  return shared ? shared->contributionRegistry.get() : nullptr;
}

std::string XsmpprojectCompletionProvider::canonical(
    std::string_view id) const {
  if (const auto* contributions = registry())
    if (const auto* contribution = contributions->resolve(id))
      return contribution->id;
  if (const auto* shared = xsmpShared())
    if (const auto alias = shared->contributionAliases.find(std::string(id));
        alias != shared->contributionAliases.end())
      return alias->second;
  return std::string(id);
}

std::string XsmpprojectCompletionProvider::preferred(
    std::string_view kind) const {
  if (const auto* contributions = registry())
    if (const auto id = contributions->preferredId(kind); !id.empty())
      return id;
  return std::string(kind);
}

std::set<std::string> XsmpprojectCompletionProvider::used(
    const Project& project, std::string_view kind) const {
  std::set<std::string> ids;
  for (const auto* element : project.elements) {
    if (const auto* profile = cursiv::ast_cast<ProfileReference>(element);
        profile && kind == "profile")
      ids.emplace(canonical(profile->profile.get()
                                ? profile->profile.get()->name
                                : profile->profile.refText));
    if (const auto* tool = cursiv::ast_cast<ToolReference>(element);
        tool && kind == "tool")
      ids.emplace(canonical(tool->tool.get() ? tool->tool.get()->name
                                             : tool->tool.refText));
  }
  return ids;
}

::lsp::CompletionList XsmpprojectCompletionProvider::getCompletion(
    const cursiv::Document& document, const ::lsp::CompletionParams& params,
    const cursiv::CancellationToken& token) const {
  auto list = DefaultCompletionProvider::getCompletion(document, params, token);
  std::ranges::stable_sort(list.items, {}, [](const auto& i) {
    return i.sortText.value_or(i.label);
  });
  return list;
}

void XsmpprojectCompletionProvider::completionFor(
    const Context& context, const cursiv::NextFeature& next,
    Accept accept) const {
  // Once for this reading of the caret.
  if (&next == context.features.data()) {
    const auto* project =
        cursiv::ast_cast<Project>(context.document.parseResult.value);
    if (project && !project->name.empty()
        && writes(context, &cursiv::ast_type_tag<Project>, "elements"))
      completeStatement(context, *project, accept);
    if (writes(context, &cursiv::ast_type_tag<Project>, "standard"))
      offerStandards(context, accept);
  }
  DefaultCompletionProvider::completionFor(context, next, accept);
}

void XsmpprojectCompletionProvider::offerStandards(const Context& context,
                                                   Accept accept) const {
  for (const auto standard : kSmpStandards) {
    cursiv::CompletionValueItem item;
    item.label = std::string(standard);
    item.kind = ::lsp::CompletionItemKind::Value;
    item.detail = "SMP standard revision.";
    item.sortText = "2000_" + item.label;
    writeQuoted(item, context, standard);
    (void)accept(context, std::move(item));
  }
}

void XsmpprojectCompletionProvider::completeStatement(const Context& context,
                                                      const Project& project,
                                                      Accept accept) const {
  // `content` is the text to insert, the label when empty.
  const auto add = [&](std::string label, std::string detail,
                       const cursiv::AstNode* node = nullptr,
                       std::string documentation = {},
                       std::string content = {}) {
    cursiv::CompletionValueItem item;
    item.label = label;
    item.filterText = label;
    item.detail = std::move(detail);
    item.kind = ::lsp::CompletionItemKind::Snippet;
    item.insertTextFormat = ::lsp::InsertTextFormat::Snippet;
    if (content.empty()) content = label;
    if (label == "Source") item.insertText = content;
    item.textEdit = buildCompletionTextEdit(context, std::move(content));
    item.sortText = std::string(label == "Source" ? "3000_" : "2500_") + label;
    if (node)
      if (const auto description = docDescription(*node))
        item.documentation = ::lsp::MarkupContent{
            .kind = ::lsp::MarkupKind::Markdown, .value = *description};
    if (!documentation.empty())
      item.documentation =
          ::lsp::MarkupContent{.kind = ::lsp::MarkupKind::Markdown,
                               .value = std::move(documentation)};
    (void)accept(context, std::move(item));
  };

  const auto* contributions = registry();
  const auto describe = [&](std::string_view kind, std::string_view id) {
    auto result = "Enable the installed XSMP " + std::string(kind) + " `"
                  + std::string(id) + "`.";
    if (contributions)
      if (const auto* contribution = contributions->resolve(id);
          contribution && !contribution->description.empty())
        result += "\n\n" + contribution->description;
    return result;
  };

  if (isPrefixOfIgnoringCase(context.identifier, "source"))
    add("Source", "Project Source Root", nullptr, {}, "source '${1:smdl}'");

  const auto usedProfiles = used(project, "profile");
  const auto usedTools = used(project, "tool");
  unsigned profiles = 0, tools = 0;
  if (contributions)
    for (const auto& [id, contribution] : contributions->all()) {
      const auto isProfile = contribution.kind == "profile";
      if ((isProfile ? usedProfiles : usedTools).contains(id)) continue;
      ++(isProfile ? profiles : tools);
      add(contribution.kind + " '" + id + "'",
          isProfile ? "Profile Reference" : "Tool Reference", nullptr,
          describe(contribution.kind, id));
    }
  else
    // With no registry, what the workspace declares.
    for (const auto& candidate : services.shared().documents->all()) {
      const auto* root = candidate->parseResult.value;
      if (const auto* profile = cursiv::ast_cast<Profile>(root);
          profile && !usedProfiles.contains(canonical(profile->name))) {
        ++profiles;
        add("profile '" + std::string(profile->name) + "'", "Profile Reference",
            profile, describe("profile", profile->name));
      }
      if (const auto* tool = cursiv::ast_cast<Tool>(root);
          tool && !usedTools.contains(canonical(tool->name))) {
        ++tools;
        add("tool '" + std::string(tool->name) + "'", "Tool Reference", tool,
            describe("tool", tool->name));
      }
    }

  if (!profiles)
    add("profile '" + preferred("profile") + "'", "Profile Reference", nullptr,
        {}, "profile '${1:" + preferred("profile") + "}'");
  if (!tools)
    add("tool '" + preferred("tool") + "'", "Tool Reference", nullptr, {},
        "tool '${1:" + preferred("tool") + "}'");

  unsigned dependencies = 0;
  if (const auto* shared = xsmpShared(); shared && shared->projectManager) {
    const auto current = shared->projectManager->getDependencies(project);
    // Any project that is not a dependency yet and would close no cycle.
    for (const auto* candidate : shared->projectManager->getProjects())
      if (!candidate->name.empty() && !current.contains(candidate)
          && !shared->projectManager->getDependencies(*candidate)
                  .contains(&project)) {
        ++dependencies;
        add("dependency '" + std::string(candidate->name) + "'",
            "Project Dependency", candidate,
            "Declare a dependency on project `" + std::string(candidate->name)
                + "`.\n\nIts visible modeling documents become available "
                  "transitively for linking and validation.\n\nThis does not "
                  "inherit the dependency project's `profile`/`tool` "
                  "declarations or contribution payload documents.");
      }
  }

  if (!dependencies)
    add("dependency '<project-name>'", "Project Dependency", nullptr, {},
        "dependency '${1:project}'");
}

void XsmpprojectCompletionProvider::completionForKeyword(
    const Context& context, const cursiv::NextFeature& next,
    std::string_view keyword, Accept accept) const {
  const auto declares =
      keyword == "project" || keyword == "profile" || keyword == "tool";
  // A statement of a project is offered with what it can name.
  if (context.node()
      && (declares || keyword == "dependency" || keyword == "source"))
    return;
  if (!declares) {
    DefaultCompletionProvider::completionForKeyword(context, next, keyword,
                                                    accept);
    return;
  }

  // The file says what it declares.
  const auto kind = documentKind(context.document.uri);
  if ((!kind.empty() && kind != keyword)
      || !isPrefixOfIgnoringCase(context.identifier, keyword))
    return;

  const auto label = kind.empty() ? std::string(keyword) : capitalized(keyword);
  cursiv::CompletionValueItem item;
  item.label = label;
  item.kind = ::lsp::CompletionItemKind::Snippet;
  item.detail =
      capitalized(keyword)
      + (kind.empty() ? (keyword == "project" ? " Definition" : " Reference")
                      : " Definition");
  item.filterText = label;
  item.sortText = "3000_" + label;
  item.insertTextFormat = ::lsp::InsertTextFormat::Snippet;
  item.insertText =
      keyword == "project"
          ? std::string("project '${1:MissionDemo}'\nsource '${2:smdl}'\n$0")
          : std::string(keyword) + " '${1:" + preferred(keyword) + "}'";
  item.textEdit = buildCompletionTextEdit(context, *item.insertText);
  (void)accept(context, std::move(item));
}

void XsmpprojectCompletionProvider::visitReferenceCandidates(
    const cursiv::ReferenceInfo& reference, const Context& context,
    cursiv::function_ref<bool(const cursiv::AstNodeDescription&)> visitor)
    const {
  const auto* project =
      cursiv::ast_cast<Project>(context.document.parseResult.value);
  const auto* shared = xsmpShared();
  if (!project || !shared || !shared->projectManager) {
    DefaultCompletionProvider::visitReferenceCandidates(reference, context,
                                                        visitor);
    return;
  }

  // Only what the project does not use yet, under its canonical name, and no
  // project that would close a dependency cycle.
  const auto* contributions = registry();
  const auto usedProfiles = used(*project, "profile");
  const auto usedTools = used(*project, "tool");
  const auto dependencies = shared->projectManager->getDependencies(*project);
  const auto refused = [&](const cursiv::AstNodeDescription& candidate) {
    const std::string name(candidate.name);
    const bool isProfile = cursiv::ast_cast<Profile>(candidate.node);
    if (isProfile || cursiv::ast_cast<Tool>(candidate.node))
      return (isProfile ? usedProfiles : usedTools).contains(canonical(name))
             || (contributions
                 && (!contributions->resolve(name) || canonical(name) != name));
    if (const auto* other = cursiv::ast_cast<Project>(candidate.node))
      return dependencies.contains(other)
             || shared->projectManager->getDependencies(*other).contains(
                 project);
    return false;
  };

  DefaultCompletionProvider::visitReferenceCandidates(
      reference, context, [&](const cursiv::AstNodeDescription& candidate) {
        return refused(candidate) || visitor(candidate);
      });
}

cursiv::CompletionValueItem
XsmpprojectCompletionProvider::createReferenceCompletionItem(
    const cursiv::AstNodeDescription& candidate,
    const cursiv::ReferenceInfo& reference, const Context& context) const {
  auto item = DefaultCompletionProvider::createReferenceCompletionItem(
      candidate, reference, context);
  item.label = std::string(candidate.name);
  writeQuoted(item, context, item.label);
  if (cursiv::ast_cast<Profile>(candidate.node))
    item.detail = "Activated XSMP profile.";
  else if (cursiv::ast_cast<Tool>(candidate.node))
    item.detail = "Enabled XSMP tool.";
  else if (cursiv::ast_cast<Project>(candidate.node))
    item.detail = "Project dependency.";
  return item;
}

}  // namespace xsmp
