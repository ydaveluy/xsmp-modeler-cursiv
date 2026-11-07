#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>
#include <xsmp/cpp/ClangFormat.hpp>
#include <xsmp/utils/FileWriter.hpp>
#include <xsmp/utils/Parallel.hpp>
#include <xsmp/utils/Process.hpp>

namespace xsmp {

namespace {

constexpr std::uint64_t hashSeed = 14695981039346656037ull;

// FNV-1a. The hashes stay within this process, and 64 bits on every platform.
std::uint64_t hashOf(std::string_view text, std::uint64_t seed = hashSeed) {
  for (const unsigned char byte : text) {
    seed ^= byte;
    seed *= 1099511628211ull;
  }
  return seed;
}

thread_local GeneratedCppBatch* currentBatch = nullptr;

// Past this much text, a batch runs before its end: memory stays bounded,
// whatever the size of a project.
constexpr std::size_t maximumPendingBytes = 8u * 1024u * 1024u;

// The tool to run, or nothing when formatting is off.
std::string toolName() {
  auto name = environmentVariable("XSMP_CLANG_FORMAT");
  if (name == "none") return {};
  return name.empty() ? "clang-format" : name;
}

// What tells `tool` apart: its name and what it says of its version. Nothing
// when it cannot run. Asked once per generation, so that a tool installed or
// updated meanwhile counts from the next one.
std::optional<std::string> toolIdentity(const std::string& tool) {
  static std::mutex mutex;
  static std::unordered_map<
      std::string, std::pair<std::uint64_t, std::optional<std::string>>>
      known;
  std::scoped_lock lock(mutex);
  const auto generation = generationNumber();
  const auto [entry, isNew] = known.try_emplace(tool);
  if (isNew || entry->second.first != generation) {
    std::optional<std::string> identity;
    const std::array<std::string, 2> command{tool, "--version"};
    try {
      if (auto result =
              runProcess(command, {}, {.timeout = std::chrono::seconds(10)});
          result.exitCode == 0)
        identity = tool + '\n' + result.output;
    } catch (const std::exception&) {
      // What cannot start is not there.
    }
    entry->second = {generation, std::move(identity)};
  }
  return entry->second.second;
}

// What the tool writes for `content`. Throws when it fails. The path goes as an
// argument of its own, for a WASI host to map it to a path of its platform.
std::string runTool(const std::string& tool, const std::filesystem::path& path,
                    std::string_view content) {
  const std::array<std::string, 3> command{tool, "-assume-filename",
                                           pathText(path)};
  auto result =
      runProcess(command, content, {.timeout = std::chrono::seconds(60)});
  if (result.exitCode != 0)
    throw std::runtime_error("exit code " + std::to_string(result.exitCode)
                             + ": " + result.errors.substr(0, 1024));
  // A tool that takes the arguments and writes nothing would empty the file.
  if (result.output.empty() && !content.empty())
    throw std::runtime_error("no output");
  return std::move(result.output);
}

// What the last run left of a file: hashes only, to save memory.
struct Memo {
  std::uint64_t input;
  std::uint64_t formatted;
};

class MemoTable {
 public:
  // The hash of what the tool last wrote for this file, if for this input.
  [[nodiscard]] std::optional<std::uint64_t> formatted(std::uint64_t key,
                                                       std::uint64_t input) {
    std::scoped_lock lock(_mutex);
    const auto entry = _entries.find(key);
    if (entry == _entries.end() || entry->second.input != input)
      return std::nullopt;
    return entry->second.formatted;
  }

  void remember(std::uint64_t key, const Memo& memo) {
    std::scoped_lock lock(_mutex);
    _entries.insert_or_assign(key, memo);
  }

 private:
  std::mutex _mutex;
  std::unordered_map<std::uint64_t, Memo> _entries;
};

MemoTable& memoTable() {
  static MemoTable table;
  return table;
}

// True when the file at `path` hashes to `formatted`.
bool isOnDisk(const std::filesystem::path& path, std::uint64_t formatted) {
  const auto disk = readFileIfPresent(path);
  return disk && hashOf(*disk) == formatted;
}

// A hash of the style files that apply in a folder and those above it: the
// nearest one decides, and it can inherit from the next. An edit of one of them
// changes what the tool writes for the same input.
class StyleHashes {
 public:
  [[nodiscard]] std::uint64_t of(const std::filesystem::path& folder) {
    if (const auto known = _known.find(folder); known != _known.end())
      return known->second;
    auto hash =
        folder.has_relative_path() ? of(folder.parent_path()) : hashSeed;
    for (const auto* name : {".clang-format", "_clang-format"})
      if (const auto content = readFileIfPresent(folder / name))
        hash = hashOf(*content, hashOf(name, hash));
    _known.emplace(folder, hash);
    return hash;
  }

 private:
  std::map<std::filesystem::path, std::uint64_t> _known;
};

}  // namespace

std::string clangFormatted(const std::filesystem::path& path,
                           std::string content) {
  const auto tool = toolName();
  if (tool.empty() || !toolIdentity(tool)) return content;
  const auto absolute = normalPath(path);
  try {
    return runTool(tool, absolute, content);
  } catch (const std::exception& error) {
    std::cerr << "warning: " << tool << " failed on " << pathText(absolute)
              << ", which stays unformatted: " << error.what() << '\n';
    return content;
  }
}

void writeGeneratedCpp(const std::filesystem::path& path, std::string content) {
  if (currentBatch)
    currentBatch->add(pathText(normalPath(path)), std::move(content), false);
  else
    writeFileIfChanged(path, clangFormatted(path, std::move(content)));
}

void writeUserCpp(const std::filesystem::path& path, std::string content) {
  if (currentBatch) {
    currentBatch->add(pathText(normalPath(path)), std::move(content), true);
    return;
  }
  // There already, it is kept as it is, and needs no formatting.
  std::error_code error;
  if (!std::filesystem::exists(std::filesystem::symlink_status(path, error)))
    content = clangFormatted(path, std::move(content));
  writeFileIfMissing(path, content);
}

GeneratedCppBatch::GeneratedCppBatch() : _previous(currentBatch) {
  currentBatch = this;
}

GeneratedCppBatch::~GeneratedCppBatch() { currentBatch = _previous; }

void GeneratedCppBatch::add(std::string path, std::string content, bool user) {
  _pendingBytes += content.size();
  const auto [entry, isNew] =
      _files.try_emplace(std::move(path), File{std::move(content), user});
  if (!isNew && !(user && entry->second.user))
    throw std::runtime_error(
        user || entry->second.user
            ? "The file '" + entry->first + "' is both generated and the user's"
            : "The generated file '" + entry->first + "' is written twice");
  if (_pendingBytes >= maximumPendingBytes) run();
}

void GeneratedCppBatch::run() {
  _pendingBytes = 0;
  const auto files = std::exchange(_files, {});
  const auto tool = toolName();
  const auto identity = tool.empty() ? std::nullopt : toolIdentity(tool);
  // A file of the user is written once, where it is missing.
  const auto write = [](const std::filesystem::path& path,
                        std::string_view content, bool user) {
    if (user)
      writeFileIfMissing(path, content);
    else
      writeFileIfChanged(path, content);
  };
  if (!identity) {
    for (const auto& [path, file] : files)
      write(pathFromText(path), file.content, file.user);
    return;
  }

  struct Pending {
    std::filesystem::path path;
    const File& file;
    std::uint64_t key;
    std::uint64_t input;
    std::string failure;
  };

  // On this thread, so that a run with nothing to do starts no thread either.
  std::vector<Pending> pending;
  StyleHashes styles;
  for (const auto& [text, file] : files) {
    auto path = pathFromText(text);
    if (file.user) {
      // There already, it is kept as it is, and needs no formatting.
      std::error_code error;
      if (std::filesystem::exists(std::filesystem::symlink_status(path, error)))
        writeFileIfMissing(path, file.content);
      else
        pending.push_back({std::move(path), file, 0, 0, {}});
      continue;
    }
    const auto key = hashOf(text);
    const auto input =
        hashOf(file.content, hashOf(*identity, styles.of(path.parent_path())));
    const auto formatted = memoTable().formatted(key, input);
    if (formatted && isOnDisk(path, *formatted))
      recordGeneratedFile(path);
    else
      pending.push_back({std::move(path), file, key, input, {}});
  }

  runInParallel(pending.size(), [&](std::size_t index) {
    auto& entry = pending[index];
    std::string formatted;
    try {
      formatted = runTool(tool, entry.path, entry.file.content);
    } catch (const std::exception& error) {
      entry.failure = error.what();
      write(entry.path, entry.file.content, entry.file.user);
      return;
    }
    write(entry.path, formatted, entry.file.user);
    if (!entry.file.user)
      memoTable().remember(entry.key, {entry.input, hashOf(formatted)});
  });

  // One warning for the run, as a broken style file fails every file.
  std::size_t failures = 0;
  const Pending* first = nullptr;
  for (const auto& file : pending) {
    if (file.failure.empty()) continue;
    if (!first) first = &file;
    ++failures;
  }
  if (first)
    std::cerr << "warning: " << tool << " failed on " << failures
              << " generated file(s), which stay unformatted. First: "
              << pathText(first->path) << ": " << first->failure << '\n';
}

}  // namespace xsmp
