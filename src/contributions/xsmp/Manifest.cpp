#include <algorithm>
#include <array>
#include <cursiv/services/ServiceRegistry.hpp>
#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/DocumentFactory.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <filesystem>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/ast.hpp>

#include "Contributions.hpp"
#include "Json.hpp"
#include "Extension.hpp"

namespace xsmp {

// The contribution API is 2.0.0. A manifest asks for it with `*`, a version
// such as `2`, `2.x` or `2.0.0`, or one of these after `^`, `~` or `>=`.
bool supportsExtensionApiVersion(std::string_view range) {
  static constexpr std::array<int, 3> api{2, 0, 0};
  static const std::regex form(
      R"(^\s*(\^|~|>=)?\s*(?:[*xX]|(\d{1,9})(?:\.(\d{1,9}|[*xX]))?(?:\.(\d{1,9}|[*xX]))?)\s*$)");
  const std::string text(range);
  std::smatch match;
  if (!std::regex_match(text, match, form)) return false;
  // -1 stands for any value of that part.
  std::array<int, 3> wanted{-1, -1, -1};
  std::array<int, 3> lowest{0, 0, 0};
  for (std::size_t i = 0; i < 3; ++i)
    if (const auto part = match[i + 2].str();
        !part.empty() && part[0] >= '0' && part[0] <= '9')
      lowest[i] = wanted[i] = std::stoi(part);
  if (lowest > api) return false;
  const auto op = match[1].str();
  if (op == ">=") return true;
  if (op == "^") return wanted[0] < 0 || wanted[0] == api[0];
  // `~` lets the last part float.
  for (std::size_t i = 0; i < (op == "~" ? 2u : 3u); ++i)
    if (wanted[i] >= 0 && wanted[i] != api[i]) return false;
  return true;
}

namespace {

// Resolves a path of a manifest. Throws when it leaves the extension root or
// names nothing.
std::filesystem::path packagePath(const std::filesystem::path& root,
                                  std::string path) {
  std::ranges::replace(path, '\\', '/');
  const auto resolved =
      std::filesystem::absolute(root / path).lexically_normal();
  if (!containsPath(root, resolved))
    throw std::invalid_argument("Contribution path is outside extension root: "
                                + path);
  if (!std::filesystem::exists(resolved))
    throw std::invalid_argument("Contribution path does not exist: "
                                + resolved.string());
  return resolved;
}

// The URI of an extension's file as a workspace document:
// `xsmp:/extensions/<extension>/<relative>`.
std::string documentUri(std::string_view extension,
                        const std::filesystem::path& relative) {
  const auto file = cursiv::path_to_file_uri(
      (std::filesystem::path("/") / "extensions" / extension / relative)
          .generic_string());
  return "xsmp:" + file.substr(std::string_view("file://").size());
}

// The handler of a manifest entry for this build, chosen as the editor client
// does: `wasiHandler` or `nativeHandler` first, else `handler`. `nativeHandler`
// is a path, or an object keyed by `<platform>-<arch>`, in the names of Node.
std::string packageHandler(const JsonObject& entry) {
#if defined(__wasi__)
  if (const auto* handler = entry.find("wasiHandler"); handler)
    return handler->string();
#else
#if defined(_WIN32)
  std::string platform = "win32-";
#elif defined(__APPLE__)
  std::string platform = "darwin-";
#else
  std::string platform = "linux-";
#endif
#if defined(__aarch64__) || defined(_M_ARM64)
  platform += "arm64";
#else
  platform += "x64";
#endif
  if (const auto* native = entry.find("nativeHandler")) {
    if (native->isString()) return native->string();
    if (native->isObject())
      if (const auto* handler = native->object().find(platform))
        return handler->string();
  }
#endif
  return stringAt(entry, "handler");
}

// The documents that the `builtins` of `entry` bring, with their URIs: a
// `builtins` path names a file, or a folder read in sorted order. Files of
// other languages are ignored.
std::vector<std::pair<std::filesystem::path, std::string>> builtinFiles(
    const JsonObject& entry, std::string_view extension,
    const std::filesystem::path& root) {
  std::vector<std::pair<std::filesystem::path, std::string>> files;
  std::set<std::string> uris;
  const auto collect = [&](const std::filesystem::path& path) {
    const auto ext = path.extension().string();
    if (ext != ".xsmpcat" && ext != ".xsmpasb" && ext != ".xsmpcfg"
        && ext != ".xsmplnk" && ext != ".xsmpsed" && ext != ".xsmpprofile"
        && ext != ".xsmptool")
      return;
    auto uri = documentUri(extension, path.lexically_relative(root));
    if (uris.insert(uri).second) files.emplace_back(path, std::move(uri));
  };
  for (const auto& value : stringsAt(entry, "builtins")) {
    const auto path = packagePath(root, value);
    if (std::filesystem::is_directory(path)) {
      std::vector<std::filesystem::path> paths;
      for (const auto& file :
           std::filesystem::recursive_directory_iterator(path))
        if (file.is_regular_file()) paths.push_back(file.path());
      std::ranges::sort(paths);
      for (const auto& file : paths) collect(file);
    } else
      collect(path);
  }
  return files;
}

}  // namespace

std::vector<std::filesystem::path> packageLibraries(
    std::span<const std::string> packages) {
  std::vector<std::filesystem::path> paths;
  for (const auto& package : packages) {
    try {
      const auto root = std::filesystem::absolute(package).lexically_normal();
      const auto metadata = ::lsp::json::parse(readText(root / "package.json"));
      for (const auto& entry : metadata.object()
                                   .get("contributes")
                                   .object()
                                   .get("xsmp")
                                   .array())
        if (entry.isObject())
          paths.push_back(packagePath(root, packageHandler(entry.object())));
    } catch (const std::exception&) {
      // Its registration reports what is wrong.
    }
  }
  return paths;
}

Json Registry::registerPackage(XsmpSharedCoreServices& shared,
                               const std::filesystem::path& packageRoot) {
  const auto root = std::filesystem::absolute(packageRoot).lexically_normal();
  const auto package = ::lsp::json::parse(readText(root / "package.json"));
  const auto& metadata = package.object();
  const auto* contributes = metadata.find("contributes");
  if (!contributes || !contributes->isObject())
    return JsonObject{{"registered", JsonArray{}}, {"failures", JsonArray{}}};
  const auto* xsmp = contributes->object().find("xsmp");
  if (!xsmp || !xsmp->isArray())
    return JsonObject{{"registered", JsonArray{}}, {"failures", JsonArray{}}};
  JsonArray entries;
  for (const auto& item : xsmp->array()) {
    // Kept, for `registerManifestEntries` to report it as a failure.
    if (!item.isObject()) {
      entries.push_back(item);
      continue;
    }
    auto entry = item.object();
    // Complete the entry as an editor client does.
    entry.insert("extensionId", stringAt(metadata, "publisher", "external")
                                    + "." + stringAt(metadata, "name"));
    entry.insert("extensionRoot", root.string());
    entry.insert("descriptorPath", stringAt(entry, "descriptor"));
    entry.insert("handlerPath", packageHandler(entry));
    entries.emplace_back(std::move(entry));
  }
  return registerManifestEntries(shared, entries);
}

std::vector<std::string> Registry::placeholders(
    const XsmpSharedCoreServices& shared, const Json& manifests) const {
  std::vector<std::string> uris;
  const auto held = [&](const std::string& uri) {
    if (shared.documents->hasDocument(uri)) uris.push_back(uri);
  };
  if (!manifests.isArray()) return uris;
  for (const auto& manifest : manifests.array()) {
    // An entry that cannot be read is for the registration to report.
    try {
      const auto& entry = manifest.object();
      const auto extension = stringAt(entry, "extensionId");
      // Its documents are already the ones of the extension.
      if (std::ranges::any_of(entries, [&](const auto& registered) {
            return registered.second.extensionId == extension;
          }))
        continue;
      const auto root =
          std::filesystem::absolute(stringAt(entry, "extensionRoot"))
              .lexically_normal();
      const auto descriptor =
          packagePath(root, stringAt(entry, "descriptorPath"));
      held(documentUri(extension, descriptor.lexically_relative(root)));
      for (const auto& [path, uri] : builtinFiles(entry, extension, root))
        held(uri);
    } catch (const std::exception&) {
    }
  }
  return uris;
}

Json Registry::registerManifestEntries(XsmpSharedCoreServices& shared,
                                       const Json& manifests) {
  if (!manifests.isArray())
    throw std::invalid_argument("Contribution manifests must be an array");
  JsonArray registered, failures;
  auto changed = false;
  for (const auto& manifest : manifests.array()) {
    // `phase` tells a failure report which step went wrong.
    std::string phase = "descriptor", id, extension, descriptorName,
                handlerName;
    try {
      const auto& entry = manifest.object();
      extension = stringAt(entry, "extensionId");
      if (extension.empty())
        throw std::invalid_argument("A contribution needs an extensionId");
      const auto root =
          std::filesystem::absolute(stringAt(entry, "extensionRoot"))
              .lexically_normal();
      descriptorName = stringAt(entry, "descriptorPath");
      handlerName = stringAt(entry, "handlerPath");
      const auto apiVersion = stringAt(entry, "apiVersion");
      if (!supportsExtensionApiVersion(apiVersion)) {
        phase = "apiVersion";
        throw std::invalid_argument("This contribution asks for API '"
                                    + apiVersion
                                    + "', and XSMP provides API 2.0.0.");
      }
      const auto descriptorPath = packagePath(root, descriptorName);
      const auto uri =
          documentUri(extension, descriptorPath.lexically_relative(root));
      auto descriptor =
          shared.documentFactory->fromString(readText(descriptorPath), uri);
      if (!descriptor->parseSucceeded())
        throw std::invalid_argument("Invalid contribution descriptor: "
                                    + descriptorPath.string());

      Contribution contribution;
      if (const auto* node =
              cursiv::ast_cast<Profile>(descriptor->parseResult.value)) {
        id = node->name;
        contribution.kind = "profile";
      } else if (const auto* node =
                     cursiv::ast_cast<Tool>(descriptor->parseResult.value)) {
        id = node->name;
        contribution.kind = "tool";
      } else
        throw std::invalid_argument(
            "Descriptor must declare a profile or tool");
      if (id.empty())
        throw std::invalid_argument("A contribution needs an identifier");
      contribution.id = id;
      contribution.label = id;
      contribution.extensionId = extension;

      // An extension may register an entry again, which changes nothing.
      if (const auto previous = entries.find(id);
          previous != entries.end()
          && previous->second.kind == contribution.kind) {
        if (previous->second.extensionId != extension)
          throw std::invalid_argument("Contribution already registered: " + id);
        registered.emplace_back(JsonObject{{"id", id},
                                           {"kind", previous->second.kind},
                                           {"extensionId", extension}});
        continue;
      }

      if (resolve(id))
        throw std::invalid_argument("Contribution name collision: " + id);

      contribution.aliases = stringsAt(entry, "aliases");
      contribution.deprecatedAliases = stringsAt(entry, "deprecatedAliases");
      for (const auto& alias : contribution.aliases)
        if (resolve(alias))
          throw std::invalid_argument("Contribution alias collision: " + alias);
      for (const auto& alias : contribution.deprecatedAliases)
        if (resolve(alias))
          throw std::invalid_argument("Contribution alias collision: " + alias);

      if (const auto* wizard = entry.find("wizard");
          wizard && wizard->isObject()) {
        contribution.label = stringAt(wizard->object(), "label", id);
        contribution.description = stringAt(wizard->object(), "description");
        contribution.defaultSelected =
            boolAt(wizard->object(), "defaultSelected");
      }

      phase = "handler";
      if (handlerName.empty())
        throw std::invalid_argument(
            "This contribution has no handler for this platform");
      extensionRegistration(packagePath(root, handlerName))(contribution,
                                                            shared);

      phase = "builtins";
      // The documents the contribution brings.
      std::vector<std::shared_ptr<cursiv::Document>> payloads;
      for (const auto& [path, uri] : builtinFiles(entry, extension, root))
        payloads.push_back(
            shared.documentFactory->fromString(readText(path), uri));

      phase = "validation";
      for (const auto& registrar : contribution.validations)
        registrar(shared, contributionCategory(shared, id));

      // A step above that fails leaves the workspace as it was. The workspace
      // changes only from here on.
      phase = "activation";
      for (const auto& alias : contribution.aliases)
        shared.contributionAliases[alias] = id;
      for (const auto& alias : contribution.deprecatedAliases) {
        shared.contributionAliases[alias] = id;
        if (alias != id
            && std::ranges::find(contribution.aliases, alias)
                   == contribution.aliases.end())
          shared.contributionDeprecatedAliases.insert(alias);
      }
      for (const auto& payload : payloads) {
        shared.contributionPayloadOwners.try_emplace(payload->uri, id);
        shared.contributionPayloadContributors[payload->uri].insert(id);
      }

      const auto kind = contribution.kind;
      add(std::move(contribution));
      if (!shared.documents->hasDocument(descriptor->uri))
        shared.documents->addDocument(std::move(descriptor));
      for (auto& payload : payloads)
        if (!shared.documents->hasDocument(payload->uri))
          shared.documents->addDocument(std::move(payload));
      changed = true;
      registered.emplace_back(
          JsonObject{{"id", id}, {"kind", kind}, {"extensionId", extension}});
    } catch (const std::exception& error) {
      JsonObject failure{{"extensionId", extension},
                         {"descriptorPath", descriptorName},
                         {"handlerPath", handlerName},
                         {"phase", phase},
                         {"message", std::string(error.what())}};
      if (!id.empty()) failure.insert("contributionId", id);
      failures.emplace_back(std::move(failure));
    }
  }

  // A new contribution can change what any document sees.
  if (changed) {
    shared.projectManager->contributionsChanged();
    std::vector<cursiv::Document*> documents;
    for (const auto& document : shared.documents->all())
      documents.push_back(document.get());
    shared.documentBuilder->build(documents, {.validation = true});
  }

  return JsonObject{{"registered", std::move(registered)},
                    {"failures", std::move(failures)}};
}

}  // namespace xsmp
