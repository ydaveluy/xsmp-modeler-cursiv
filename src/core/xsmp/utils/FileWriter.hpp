#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace xsmp {

/// `path` as UTF-8, whatever the platform stores.
[[nodiscard]] std::string pathText(const std::filesystem::path& path);

/// The path that the UTF-8 `text` names.
[[nodiscard]] std::filesystem::path pathFromText(std::string_view text);

/// `path` made absolute and normal, the form in which generated files compare.
[[nodiscard]] std::filesystem::path normalPath(
    const std::filesystem::path& path);

/// True when `path` names a regular file that already exists.
[[nodiscard]] bool fileExists(const std::filesystem::path& path) noexcept;

/// The content of the regular file at `path`, or nothing if it cannot be read.
[[nodiscard]] std::optional<std::string> readFileIfPresent(
    const std::filesystem::path& path);

/// Writes `content` to `path` (creating parent directories as needed) unless
/// the file already has that content, so that an unchanged file keeps its time.
/// Returns whether a write actually happened.
///
/// While a generation runs, this notes the file for its `GeneratedFilesRecord`.
/// Inside its generated folders, it also removes what stands in the way: a file
/// where a folder goes, or a folder where the file goes.
bool writeFileIfChanged(const std::filesystem::path& path,
                        std::string_view content);

/// Writes `content` to `path` only when no file is there: a file the user owns
/// once it exists. Returns whether a write actually happened.
///
/// While a generation runs, this notes the file as the user's, which a
/// generated folder cannot hold.
bool writeFileIfMissing(const std::filesystem::path& path,
                        std::string_view content);

/// The files of one generation. While this exists, each file that
/// `writeFileIfChanged` or `writeFileIfMissing` gets is noted, written or not,
/// from any thread. A generator that keeps a file without calling them uses
/// `recordGeneratedFile`.
///
/// A generated file noted twice, or noted as the user's too, is an error: two
/// generators would write it. A file of the user noted twice is not, as the
/// second finds it there.
class GeneratedFilesRecord {
 public:
  /// `folders` are the generated folders of this generation.
  explicit GeneratedFilesRecord(std::span<const std::filesystem::path> folders);
  ~GeneratedFilesRecord();
  GeneratedFilesRecord(const GeneratedFilesRecord&) = delete;
  GeneratedFilesRecord& operator=(const GeneratedFilesRecord&) = delete;

  /// The generated folders, absolute and normal.
  [[nodiscard]] const std::vector<std::filesystem::path>& folders() const {
    return _folders;
  }

  /// Whether this generation noted `path`, under that name or, where a
  /// filesystem ignores case, under another name of the same file.
  [[nodiscard]] bool noted(const std::filesystem::path& path) const;

  /// The generated files noted under `folder`, relative to it.
  [[nodiscard]] std::vector<std::filesystem::path> generatedUnder(
      const std::filesystem::path& folder) const;

  /// Whether this generation noted `path` as a file of the user.
  [[nodiscard]] bool isUserFile(const std::filesystem::path& path) const;

  /// To call once all of a generation has succeeded. Removes from the generated
  /// folders each file not noted here, then the folders left empty, except the
  /// generated folders themselves. Follows no link and touches nothing outside
  /// them. Throws, once all is tried, for the files that cannot be removed: a
  /// stale source can break a build. A folder that stays is no error, as a
  /// process may hold it.
  void removeStaleFiles() const;

 private:
  friend void noteFile(const std::filesystem::path&, bool);

  std::vector<std::filesystem::path> _folders;
  // Normal path text, and whether the file is the user's.
  std::unordered_map<std::string, bool> _files;
};

/// Notes a file that generation produces for the record that is running, if
/// any.
void recordGeneratedFile(const std::filesystem::path& path);

/// A number that each new `GeneratedFilesRecord` changes, for what is checked
/// once per generation, such as the version of a tool.
[[nodiscard]] std::uint64_t generationNumber() noexcept;

/// To call before generating into `folders`, so that no write goes through a
/// link out of them: removes a link or a file at the path of a folder, and each
/// link or special file inside. Follows no link. Every entry is tried, then
/// this throws for those that failed.
void prepareGeneratedFolders(std::span<const std::filesystem::path> folders);

}  // namespace xsmp
