#include "Contributions.hpp"

#include <algorithm>
#include <cctype>
#include <cursiv/services/ServiceRegistry.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <iostream>
#include <ranges>
#include <set>
#include <stdexcept>
#include <xsmp/Json.hpp>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/XsmpcatCoreServices.hpp>
#include <xsmp/utils/FileWriter.hpp>

namespace xsmp {

void Registry::add(Contribution contribution) {
  if (contribution.id.empty())
    throw std::invalid_argument("A contribution needs an identifier");
  std::set<std::string_view> identifiers{contribution.id};
  for (const auto& alias : contribution.aliases)
    if (!alias.empty()) identifiers.insert(alias);
  for (const auto& alias : contribution.deprecatedAliases)
    if (!alias.empty()) identifiers.insert(alias);
  for (const auto& [id, existing] : entries) {
    if (identifiers.contains(id))
      throw std::invalid_argument("Duplicate XSMP contribution: " + id);
    for (const auto& alias : existing.aliases)
      if (identifiers.contains(alias))
        throw std::invalid_argument("Duplicate XSMP contribution alias: "
                                    + alias);
    for (const auto& alias : existing.deprecatedAliases)
      if (identifiers.contains(alias))
        throw std::invalid_argument("Duplicate XSMP contribution alias: "
                                    + alias);
  }

  // What a generated or tracked folder holds can be removed, so it stays
  // inside the output, and out of the list that `.xsmp` keeps.
  const auto check = [&](std::string_view kind, const std::string& folder) {
    const auto normal = pathFromText(folder).lexically_normal();
    auto first = normal.empty() ? std::string{} : pathText(*normal.begin());
    std::ranges::transform(first, first.begin(), [](unsigned char c) {
      return static_cast<char>(std::tolower(c));
    });
    if (normal.empty() || normal == "." || normal.has_root_path()
        || first == ".." || first == ".xsmp")
      throw std::invalid_argument("Invalid " + std::string(kind) + " folder '"
                                  + folder + "' of XSMP contribution "
                                  + contribution.id);
  };
  for (const auto& folder : contribution.generatedFolders)
    check("generated", folder);
  for (const auto& tracked : contribution.trackedFolders) {
    check("tracked", tracked.folder);
    if (tracked.generator.empty() || tracked.output.empty())
      throw std::invalid_argument("A tracked folder of XSMP contribution "
                                  + contribution.id
                                  + " needs a generator and an output name");
  }

  if (contribution.kind.empty()) contribution.kind = "tool";
  if (contribution.label.empty()) contribution.label = contribution.id;
  if (contribution.extensionId.empty())
    contribution.extensionId = "xsmp.builtin." + contribution.id;
  const auto id = contribution.id;
  entries.emplace(id, std::move(contribution));
  order.push_back(id);
}

const Contribution* Registry::resolve(std::string_view id) const noexcept {
  if (id.empty()) return nullptr;
  if (const auto entry = entries.find(id); entry != entries.end())
    return &entry->second;
  for (const auto& entry : std::views::values(entries))
    if (std::ranges::find(entry.aliases, id) != entry.aliases.end()
        || std::ranges::find(entry.deprecatedAliases, id)
               != entry.deprecatedAliases.end())
      return &entry;
  return nullptr;
}

const Contribution& Registry::get(std::string_view id) const {
  if (const auto* entry = resolve(id)) return *entry;
  throw std::invalid_argument("Unknown XSMP contribution: " + std::string(id));
}

Registry& registry(XsmpSharedCoreServices& shared) {
  if (!shared.contributionRegistry) {
    const auto& services = dynamic_cast<const XsmpcatCoreServices&>(
        shared.serviceRegistry->getServices({}, "xsmpcat"));
    shared.contributionRegistry =
        std::make_shared<Registry>(makeBuiltinRegistry(services));
  }
  return *shared.contributionRegistry;
}

Registry& registry(const cursiv::SharedCoreServices& shared) {
  return registry(const_cast<XsmpSharedCoreServices&>(
      dynamic_cast<const XsmpSharedCoreServices&>(shared)));
}

void ScaffoldContext::writeFile(const std::filesystem::path& path,
                                std::string_view text) const {
  writeFileIfChanged(path, text);
}

void ScaffoldContext::addDependency(std::string name) {
  if (std::ranges::find(dependencies, name) == dependencies.end())
    dependencies.push_back(std::move(name));
}

Json ScaffoldContext::getPromptValue(std::string_view id) const {
  if (promptValues.isObject())
    if (const auto* value =
            promptValues.object().find(contributionId + "." + std::string(id)))
      return *value;
  return {};
}

Json Registry::summaries(std::string_view kind) const {
  JsonArray result;
  for (const auto& id : order) {
    const auto& entry = entries.at(id);
    if (!kind.empty() && entry.kind != kind) continue;
    JsonObject value{{"id", id},
                     {"kind", entry.kind},
                     {"label", entry.label},
                     {"defaultSelected", entry.defaultSelected},
                     {"extensionId", entry.extensionId},
                     {"hasScaffolder", static_cast<bool>(entry.scaffolder)}};
    if (!entry.description.empty())
      value.insert("description", entry.description);
    result.emplace_back(std::move(value));
  }
  return result;
}

std::string Registry::preferredId(std::string_view kind) const {
  std::string first;
  for (const auto& id : order) {
    const auto& entry = entries.at(id);
    if (entry.kind != kind) continue;
    if (entry.defaultSelected) return id;
    if (first.empty()) first = id;
  }
  return first;
}

Json Registry::wizardPrompts(std::string_view profile,
                             std::span<const std::string> tools) const {
  JsonArray result;
  const auto append = [&](std::string_view id, std::string_view kind) {
    const auto* entry = resolve(id);
    if (!entry || entry->kind != kind) return;
    for (const auto& prompt : entry->wizardPrompts) {
      auto value = prompt.object();
      value.insert("contributionId", entry->id);
      value.insert("key", entry->id + "." + stringAt(value, "id"));
      result.emplace_back(std::move(value));
    }
  };
  if (!profile.empty()) append(profile, "profile");
  for (const auto& id : tools) append(id, "tool");
  return result;
}

Json Registry::scaffoldProject(ScaffoldContext context) const {
  JsonArray failures;
  const auto scaffold = [&](std::string_view id, std::string_view kind) {
    const auto* entry = resolve(id);
    if (!entry || entry->kind != kind) return;
    context.contributionId = entry->id;
    try {
      if (entry->scaffolder) entry->scaffolder(context);
    } catch (const std::exception& error) {
      failures.emplace_back(JsonObject{{"contributionId", entry->id},
                                       {"message", std::string(error.what())}});
    }
  };
  if (!context.selectedProfileId.empty())
    scaffold(context.selectedProfileId, "profile");
  for (const auto& id : context.selectedToolIds) scaffold(id, "tool");
  JsonArray dependencies;
  for (const auto& name : context.dependencies) dependencies.emplace_back(name);
  return JsonObject{{"dependencies", std::move(dependencies)},
                    {"failures", std::move(failures)}};
}

void loadBuiltinDescriptors(cursiv::SharedCoreServices& shared) {
  const auto& contributions = registry(shared);
  auto& xsmpShared = dynamic_cast<XsmpSharedCoreServices&>(shared);
  std::vector<cursiv::Document*> documents;
  // The same ids as `makeBuiltinRegistry`.
  for (const auto id : {"xsmp-sdk", "esa-cdk", "smp", "adoc", "python"}) {
    const auto& contribution = contributions.get(id);
    // Before the build below, whose descriptors export these names too.
    for (const auto& alias : contribution.deprecatedAliases) {
      xsmpShared.contributionAliases[alias] = id;
      xsmpShared.contributionDeprecatedAliases.insert(alias);
    }
    const auto tool = contribution.kind == "tool";
    const auto uri = "xsmp:/contributions/" + std::string(id)
                     + (tool ? ".xsmptool" : ".xsmpprofile");
    if (!shared.documents->hasDocument(uri))
      documents.push_back(
          shared.documents
              ->createDocument(
                  uri, std::string(tool ? "tool '" : "profile '") + id + "'\n")
              .get());
  }
  shared.documentBuilder->build(documents, {.validation = false});
}

}  // namespace xsmp
