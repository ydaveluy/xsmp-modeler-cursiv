#include <algorithm>
#include <cctype>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>
#include <system_error>
#include <tuple>
#include <xsmp/Json.hpp>
#include <xsmp/utils/FileWriter.hpp>

#include "Contributions.hpp"

namespace xsmp {

namespace {

namespace fs = std::filesystem;

std::string lowerCase(std::string text) {
  std::ranges::transform(text, text.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return text;
}

// The parts of a relative path, without the empty ones and `.`. Nothing when
// the path is absolute or goes up with `..`.
std::optional<std::vector<std::string>> relativeParts(std::string_view value) {
  std::string text(value);
  std::ranges::replace(text, '\\', '/');
  if (text.find('\0') != std::string::npos || text.starts_with('/')
      || (text.size() > 2 && std::isalpha(static_cast<unsigned char>(text[0]))
          && text[1] == ':' && text[2] == '/'))
    return std::nullopt;
  std::vector<std::string> parts;
  for (const auto part : std::views::split(text, '/')) {
    std::string name(part.begin(), part.end());
    if (name == "..") return std::nullopt;
    if (!name.empty() && name != ".") parts.push_back(std::move(name));
  }
  return parts;
}

bool insideTheList(std::span<const std::string> parts) {
  return !parts.empty() && lowerCase(parts.front()) == ".xsmp";
}

// The files that generation wrote in the tracked folders of one output
// directory, by the generator, output and folder that own them. Kept in
// `.xsmp/generated-files.json`, written as the other XSMP tools write it, so
// that one reads what the other wrote.
class Inventory {
 public:
  using Key = std::tuple<std::string, std::string, std::string>;

  explicit Inventory(const fs::path& directory)
      : _file(directory / ".xsmp" / "generated-files.json") {
    _previous = readFileIfPresent(_file);
    if (!_previous) return;
    try {
      read(::lsp::json::parse(*_previous));
    } catch (const std::exception&) {
      outputs.clear();
      std::cerr << "warning: Ignoring invalid generated-file inventory '"
                << pathText(_file) << "'; unknown files will be preserved.\n";
    }
  }

  // Writes the list where it changed, through a file of its own that takes its
  // place, so that it is never half written.
  void save() {
    // No list yet and nothing to put in one: none is written.
    if (!_previous && std::ranges::all_of(outputs, [](const auto& output) {
          return output.second.empty();
        }))
      return;
    std::string content = R"({"version":1,"outputs":[)";
    auto firstOutput = true;
    for (const auto& [key, files] : outputs) {
      if (files.empty()) continue;
      if (!firstOutput) content += ',';
      firstOutput = false;
      const auto& [owner, output, directory] = key;
      content += R"({"owner":)" + quoted(owner) + R"(,"output":)"
                 + quoted(output) + R"(,"directory":)" + quoted(directory)
                 + R"(,"files":[)";
      auto firstFile = true;
      for (const auto& file : files) {
        if (!firstFile) content += ',';
        firstFile = false;
        content += quoted(file);
      }
      content += "]}";
    }
    content += "]}\n";
    if (content == _previous) return;

    auto temporary = _file;
    temporary +=
        "."
        + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count())
        + ".tmp";
    std::error_code error;
    fs::create_directories(_file.parent_path(), error);
    {
      std::ofstream stream(temporary, std::ios::binary | std::ios::trunc);
      stream << content;
      stream.close();
      if (!error && stream)
        fs::rename(temporary, _file, error);
      else if (!error)
        error = std::make_error_code(std::errc::io_error);
    }
    if (error) {
      std::error_code ignored;
      fs::remove(temporary, ignored);
      throw std::runtime_error("Cannot write '" + pathText(_file)
                               + "': " + error.message());
    }
    _previous = std::move(content);
  }

  std::map<Key, std::set<std::string>> outputs;

 private:
  void read(const Json& value) {
    const auto& root = value.object();
    const auto* version = root.find("version");
    const auto* list = root.find("outputs");
    if (!version || !version->isInteger() || version->integer() != 1 || !list)
      throw std::invalid_argument("unknown inventory");
    for (const auto& item : list->array()) {
      const auto& entry = item.object();
      const auto text = [&](std::string_view name) -> const std::string& {
        const auto* field = entry.find(name);
        if (!field) throw std::invalid_argument("missing field");
        return field->string();
      };
      Key key{text("owner"), text("output"), text("directory")};
      const auto directory = relativeParts(std::get<2>(key));
      const auto* names = entry.find("files");
      if (!directory || !names) throw std::invalid_argument("bad output");
      std::set<std::string> files;
      for (const auto& name : names->array()) {
        auto parts = relativeParts(name.string());
        if (!parts || parts->empty()) throw std::invalid_argument("bad file");
        parts->insert(parts->begin(), directory->begin(), directory->end());
        if (insideTheList(*parts)) throw std::invalid_argument("bad file");
        files.insert(name.string());
      }
      if (!outputs.emplace(std::move(key), std::move(files)).second)
        throw std::invalid_argument("duplicate output");
    }
  }

  // As JSON writes a string: the quote, the backslash and the control
  // characters escaped, the rest as it is.
  static std::string quoted(std::string_view text) {
    std::string out = "\"";
    for (const auto c : text) {
      switch (c) {
        case '"':
          out += "\\\"";
          break;
        case '\\':
          out += "\\\\";
          break;
        case '\b':
          out += "\\b";
          break;
        case '\f':
          out += "\\f";
          break;
        case '\n':
          out += "\\n";
          break;
        case '\r':
          out += "\\r";
          break;
        case '\t':
          out += "\\t";
          break;
        default:
          if (static_cast<unsigned char>(c) < 0x20) {
            constexpr std::string_view hex = "0123456789abcdef";
            out += "\\u00";
            out += hex[static_cast<unsigned char>(c) >> 4];
            out += hex[static_cast<unsigned char>(c) & 0xF];
          } else {
            out += c;
          }
      }
    }
    return out + '"';
  }

  fs::path _file;
  std::optional<std::string> _previous;
};

// A tracked folder of one output directory, under the name of its owner.
struct Tracked {
  std::string owner;
  TrackedFolder folder;
};

}  // namespace

void Registry::generate(std::span<const GenerationGroup> groups) const {
  std::vector<fs::path> generatedFolders;
  std::map<fs::path, std::vector<Tracked>> trackedByDirectory;
  for (const auto& group : groups) {
    const auto directory = normalPath(group.directory);
    auto& tracked = trackedByDirectory[directory];
    std::set<std::string_view> seen;
    for (const auto& id : group.identifiers) {
      const auto& contribution = get(id);
      if (!seen.insert(contribution.id).second) continue;
      for (const auto& folder : contribution.generatedFolders)
        generatedFolders.push_back(directory / pathFromText(folder));
      for (const auto& folder : contribution.trackedFolders)
        tracked.push_back({contribution.id + "/" + folder.generator, folder});
    }
  }

  // Cleaning a generated folder would remove the files of the user in it.
  const auto lowered = [](const fs::path& path) {
    return pathFromText(lowerCase(pathText(normalPath(path))));
  };
  for (const auto& [directory, tracked] : trackedByDirectory)
    for (const auto& [owner, folder] : tracked) {
      const auto path = lowered(directory / pathFromText(folder.folder));
      for (const auto& generated : generatedFolders) {
        const auto root = lowered(generated);
        if (std::mismatch(root.begin(), root.end(), path.begin(), path.end())
                .first
            == root.end())
          throw std::invalid_argument("The tracked folder '" + folder.folder
                                      + "' of " + owner
                                      + " is in a generated folder");
      }
    }

  GeneratedFilesRecord record(generatedFolders);
  prepareGeneratedFolders(record.folders());
  std::map<fs::path, Inventory> inventories;
  for (const auto& [directory, tracked] : trackedByDirectory)
    inventories.try_emplace(directory, directory);

  for (const auto& group : groups)
    generateBatch(group.nodes, group.directory, group.identifiers);

  // All has succeeded: what is stale goes. Each step is tried, and the lists
  // are kept, before what failed is reported.
  std::string failures;
  const auto fail = [&](std::string message) {
    if (!failures.empty()) failures += '\n';
    failures += message;
  };
  try {
    record.removeStaleFiles();
  } catch (const std::exception& error) {
    fail(error.what());
  }
  for (auto& [directory, inventory] : inventories) {
    for (const auto& [owner, folder] : trackedByDirectory.at(directory)) {
      const auto root = directory / pathFromText(folder.folder);
      auto& files = inventory.outputs[{owner, folder.output, folder.folder}];
      std::set<std::string> kept;
      for (const auto& file : files) {
        const auto path = root / pathFromText(file);
        if (record.noted(path)) {
          kept.insert(file);
          continue;
        }
        // Gone, or no longer a file: nothing of generation to remove.
        std::error_code error;
        if (!fs::is_regular_file(fs::symlink_status(path, error))) continue;
        if (!fs::remove(path, error) && error) {
          fail("Cannot remove generated path '" + pathText(path)
               + "': " + error.message());
          kept.insert(file);
        }
      }
      // The list separates folders with `/` on every system, as the other
      // XSMP tools write it.
      for (const auto& file : record.generatedUnder(root)) {
        const auto text = file.generic_u8string();
        kept.emplace(reinterpret_cast<const char*>(text.data()), text.size());
      }
      files = std::move(kept);
    }
    // A file of the user leaves every list: generation no longer owns it.
    for (auto& [key, files] : inventory.outputs)
      std::erase_if(files, [&](const std::string& file) {
        return record.isUserFile(directory / pathFromText(std::get<2>(key))
                                 / pathFromText(file));
      });
    try {
      inventory.save();
    } catch (const std::exception& error) {
      fail(error.what());
    }
  }
  if (!failures.empty()) throw std::runtime_error(failures);
}

}  // namespace xsmp
