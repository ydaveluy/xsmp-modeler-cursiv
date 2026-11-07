#include <algorithm>
#include <atomic>
#include <cerrno>
#include <fstream>
#include <ios>
#include <iterator>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unordered_map>
#include <xsmp/utils/FileWriter.hpp>

namespace xsmp {

namespace fs = std::filesystem;

std::string pathText(const fs::path& path) {
  const auto value = path.u8string();
  return {reinterpret_cast<const char*>(value.data()), value.size()};
}

fs::path pathFromText(std::string_view text) {
  return std::u8string_view(reinterpret_cast<const char8_t*>(text.data()),
                            text.size());
}

fs::path normalPath(const fs::path& path) {
  return fs::absolute(path).lexically_normal();
}

bool fileExists(const fs::path& path) noexcept {
  std::error_code error;
  return fs::is_regular_file(path, error);
}

std::optional<std::string> readFileIfPresent(const fs::path& path) {
  std::error_code error;
  if (!fs::is_regular_file(path, error)) return std::nullopt;
  std::ifstream stream(path, std::ios::binary);
  if (!stream.is_open()) {
    return std::nullopt;
  }
  std::ostringstream buffer;
  buffer << stream.rdbuf();
  if (stream.bad()) return std::nullopt;
  return std::move(buffer).str();
}

namespace {

// The records of the generations that are running. There is normally at most
// one. If they overlap, each keeps what the others write, which is the safe
// way.
struct Running {
  std::mutex mutex;
  std::vector<GeneratedFilesRecord*> records;
};

Running& running() {
  static Running state;
  return state;
}

// What could not be removed, for a report once all has been tried.
using Failures = std::vector<std::string>;

void addFailure(Failures& failures, const fs::path& path,
                const std::error_code& error) {
  failures.push_back("Cannot remove generated path '" + pathText(path)
                     + "': " + error.message());
}

std::vector<fs::path> entriesOf(const fs::path& folder, Failures& failures) {
  std::vector<fs::path> entries;
  std::error_code error;
  for (fs::directory_iterator it(folder, error), end; !error && it != end;
       it.increment(error))
    entries.push_back(it->path());
  if (error) addFailure(failures, folder, error);
  return entries;
}

bool removeEntry(const fs::path& path, Failures& failures) {
  std::error_code error;
  fs::remove(path, error);
  if (error) addFailure(failures, path, error);
  return !error;
}

// Removes `path` and what it holds. Deleting the children first follows no link
// at any depth. `remove_all` also fails on a link to a folder under Node's
// WASI.
void removeTree(const fs::path& path, Failures& failures) {
  std::error_code error;
  if (fs::is_directory(fs::symlink_status(path, error)))
    for (const auto& child : entriesOf(path, failures))
      removeTree(child, failures);
  removeEntry(path, failures);
}

// Removes from `folder` each entry that is neither a folder nor a regular file:
// a write would go through a link, and a pipe would never return.
void removeLinks(const fs::path& folder, Failures& failures) {
  for (const auto& entry : entriesOf(folder, failures)) {
    std::error_code error;
    const auto status = fs::symlink_status(entry, error);
    if (fs::is_directory(status))
      removeLinks(entry, failures);
    else if (!fs::is_regular_file(status))
      removeEntry(entry, failures);
  }
}

// Removes from `folder` what `isKept` refuses, then each folder left empty.
// Returns whether `folder` ends up empty. A folder that stays is no failure: a
// process may hold it, and a file in it that stays is reported already.
template <class IsKept>
bool removeUnkept(const fs::path& folder, const IsKept& isKept,
                  Failures& failures) {
  auto empty = true;
  for (const auto& entry : entriesOf(folder, failures)) {
    std::error_code error;
    auto gone = false;
    if (fs::is_directory(fs::symlink_status(entry, error))) {
      Failures ignored;
      gone =
          removeUnkept(entry, isKept, failures) && removeEntry(entry, ignored);
    } else {
      gone = !isKept(entry) && removeEntry(entry, failures);
    }
    empty = empty && gone;
  }
  return empty;
}

std::string lowerCase(std::string text) {
  for (auto& character : text)
    if (character >= 'A' && character <= 'Z') character += 'a' - 'A';
  return text;
}

bool isInside(const fs::path& folder, const fs::path& path) {
  const auto [left, right] =
      std::mismatch(folder.begin(), folder.end(), path.begin(), path.end());
  return left == folder.end() && right != path.end();
}

// The generated folder of a running generation that holds `normal`, if any.
std::optional<fs::path> generatedFolderOf(const fs::path& normal) {
  auto& state = running();
  std::scoped_lock lock(state.mutex);
  for (const auto* record : state.records)
    for (const auto& folder : record->folders())
      if (isInside(folder, normal)) return folder;
  return std::nullopt;
}

// Removes what stands in the way of a write to `path`, in a generated folder of
// a running generation. Returns false outside those, where nothing is removed.
bool removeObstacles(const fs::path& path) {
  const auto normal = normalPath(path);
  auto current = generatedFolderOf(normal);
  if (!current) return false;

  Failures ignored;  // The write that follows reports its own failure.
  const auto parts = normal.lexically_relative(*current);
  for (auto part = parts.begin(); part != parts.end(); ++part) {
    *current /= *part;
    std::error_code error;
    const auto status = fs::symlink_status(*current, error);
    if (!fs::exists(status)) break;
    // Only the last part is a file.
    const auto isFile = std::next(part) == parts.end();
    if (fs::is_directory(status) == isFile) {
      removeTree(*current, ignored);
      break;
    }
  }
  return true;
}

// Writes `content` to `path`, whatever is there.
void writeFile(const fs::path& path, std::string_view content) {
  const auto fail = [&](const std::error_code& error) {
    throw std::runtime_error("Error generating file '" + path.string()
                             + "': " + error.message());
  };

  std::ofstream stream;
  const auto open = [&]() -> std::error_code {
    std::error_code error;
    if (!path.parent_path().empty())
      fs::create_directories(path.parent_path(), error);
    if (error) return error;
    stream.open(path, std::ios::binary | std::ios::trunc);
    if (!stream.is_open()) return {errno, std::generic_category()};
    return {};
  };

  auto error = open();
  if (error) {
    // One at a time: writes beside this one may have the same obstacle, and one
    // must not remove the folder that another has just put in its place.
    static std::mutex mutex;
    std::scoped_lock lock(mutex);
    if (removeObstacles(path)) error = open();
  }
  if (error) fail(error);
  stream << content;
  stream.flush();
  if (!stream) fail({errno, std::generic_category()});
  stream.close();
  if (!stream) fail({errno, std::generic_category()});
}

std::atomic<std::uint64_t> generations{0};

}  // namespace

// Declared a friend of the record, for the functions below.
void noteFile(const fs::path& path, bool user) {
  auto& state = running();
  {
    std::scoped_lock lock(state.mutex);
    if (state.records.empty()) return;
  }
  auto text = pathText(normalPath(path));
  std::scoped_lock lock(state.mutex);
  for (auto* record : state.records) {
    const auto [entry, isNew] = record->_files.try_emplace(text, user);
    if (isNew || (user && entry->second)) continue;
    throw std::runtime_error(
        user || entry->second
            ? "The file '" + text + "' is both generated and the user's"
            : "The generated file '" + text + "' is written twice");
  }
}

bool writeFileIfChanged(const fs::path& path, std::string_view content) {
  noteFile(path, false);
  if (const auto existing = readFileIfPresent(path);
      existing.has_value() && *existing == content) {
    return false;
  }
  writeFile(path, content);
  return true;
}

bool writeFileIfMissing(const fs::path& path, std::string_view content) {
  if (const auto folder = generatedFolderOf(normalPath(path)))
    throw std::runtime_error("The file '" + pathText(path)
                             + "' of the user cannot be in the generated "
                               "folder '"
                             + pathText(*folder) + "'");
  noteFile(path, true);
  std::error_code error;
  const auto status = fs::symlink_status(path, error);
  if (fs::is_regular_file(status) || fs::is_symlink(status)) return false;
  if (fs::exists(status))
    throw std::runtime_error("The file of the user '" + pathText(path)
                             + "' is not a file");
  writeFile(path, content);
  return true;
}

GeneratedFilesRecord::GeneratedFilesRecord(std::span<const fs::path> folders) {
  for (const auto& folder : folders) _folders.push_back(normalPath(folder));
  ++generations;
  auto& state = running();
  std::scoped_lock lock(state.mutex);
  state.records.push_back(this);
}

GeneratedFilesRecord::~GeneratedFilesRecord() {
  auto& state = running();
  std::scoped_lock lock(state.mutex);
  std::erase(state.records, this);
}

bool GeneratedFilesRecord::noted(const fs::path& path) const {
  const auto text = pathText(normalPath(path));
  std::scoped_lock lock(running().mutex);
  if (_files.contains(text)) return true;
  // A filesystem may ignore case: a type renamed that way then writes to the
  // file of its old name, which the record knows by the new one.
  const auto lower = lowerCase(text);
  for (const auto& [file, user] : _files) {
    std::error_code error;
    if (lowerCase(file) == lower
        && fs::equivalent(path, pathFromText(file), error))
      return true;
  }
  return false;
}

std::vector<fs::path> GeneratedFilesRecord::generatedUnder(
    const fs::path& folder) const {
  const auto normal = normalPath(folder);
  std::vector<fs::path> files;
  std::scoped_lock lock(running().mutex);
  for (const auto& [file, user] : _files)
    if (const auto path = pathFromText(file); !user && isInside(normal, path))
      files.push_back(path.lexically_relative(normal));
  return files;
}

bool GeneratedFilesRecord::isUserFile(const fs::path& path) const {
  std::scoped_lock lock(running().mutex);
  const auto entry = _files.find(pathText(normalPath(path)));
  return entry != _files.end() && entry->second;
}

void GeneratedFilesRecord::removeStaleFiles() const {
  Failures failures;
  {
    std::scoped_lock lock(running().mutex);
    // The other names of the noted files, where a filesystem ignores case.
    std::unordered_map<std::string, std::string> byLowerCase;
    const auto isKept = [&](const fs::path& entry) {
      const auto text = pathText(entry);
      if (_files.contains(text)) return true;
      if (byLowerCase.empty())
        for (const auto& [file, user] : _files)
          byLowerCase.emplace(lowerCase(file), file);
      const auto same = byLowerCase.find(lowerCase(text));
      std::error_code error;
      return same != byLowerCase.end()
             && fs::equivalent(entry, pathFromText(same->second), error);
    };
    for (const auto& folder : _folders) {
      std::error_code error;
      if (fs::is_directory(fs::symlink_status(folder, error)))
        removeUnkept(folder, isKept, failures);
    }
  }
  if (failures.empty()) return;
  std::string message;
  for (const auto& failure : failures) {
    if (!message.empty()) message += '\n';
    message += failure;
  }
  throw std::runtime_error(message);
}

void recordGeneratedFile(const fs::path& path) { noteFile(path, false); }

std::uint64_t generationNumber() noexcept { return generations.load(); }

void prepareGeneratedFolders(std::span<const fs::path> folders) {
  Failures failures;
  for (const auto& folder : folders) {
    const auto normal = normalPath(folder);
    std::error_code error;
    const auto status = fs::symlink_status(normal, error);
    if (error == std::errc::no_such_file_or_directory) continue;
    if (error)
      addFailure(failures, normal, error);
    else if (fs::is_directory(status))
      removeLinks(normal, failures);
    else if (status.type() != fs::file_type::not_found)
      removeEntry(normal, failures);
  }
  if (failures.empty()) return;
  std::string message;
  for (const auto& failure : failures) {
    if (!message.empty()) message += '\n';
    message += failure;
  }
  throw std::runtime_error(message);
}

}  // namespace xsmp
