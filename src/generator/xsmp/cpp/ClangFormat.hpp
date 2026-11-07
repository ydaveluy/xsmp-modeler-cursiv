#pragma once

#include <cstddef>
#include <filesystem>
#include <map>
#include <string>

namespace xsmp {

/// `content` as clang-format writes it for a file at `path`: the nearest
/// `.clang-format` above that path decides the style. Without the tool, or with
/// `XSMP_CLANG_FORMAT` set to `none`, the content stays as it is. So it does
/// when the tool fails, with a warning on stderr. `XSMP_CLANG_FORMAT` names
/// another tool, such as `clang-format-18`.
[[nodiscard]] std::string clangFormatted(const std::filesystem::path& path,
                                         std::string content);

/// Formats `content` as `clangFormatted` does and writes it to `path`, unless
/// the file already has that content. With a `GeneratedCppBatch` open on this
/// thread, this waits for its `run`.
void writeGeneratedCpp(const std::filesystem::path& path, std::string content);

/// The same for a file of the user: formatted and written only where it is
/// missing, as `writeFileIfMissing` does.
void writeUserCpp(const std::filesystem::path& path, std::string content);

/// While this exists, `writeGeneratedCpp` and `writeUserCpp` on this thread
/// only collect their files. `run` formats and writes them together, on several
/// threads, as a process per file adds up. Files that are not run are dropped.
/// A batch holding a lot of text runs early, so that memory stays bounded. A
/// generated file given twice throws.
///
/// `run` starts no process for a file that is unchanged since this process last
/// wrote it: same content from the generator, same version of the tool, same
/// style files, and what it wrote still on disk. The version is asked once per
/// generation.
class GeneratedCppBatch {
 public:
  GeneratedCppBatch();
  ~GeneratedCppBatch();
  GeneratedCppBatch(const GeneratedCppBatch&) = delete;
  GeneratedCppBatch& operator=(const GeneratedCppBatch&) = delete;

  void run();

 private:
  friend void writeGeneratedCpp(const std::filesystem::path&, std::string);
  friend void writeUserCpp(const std::filesystem::path&, std::string);

  struct File {
    std::string content;
    bool user;
  };

  void add(std::string path, std::string content, bool user);

  GeneratedCppBatch* _previous;
  std::size_t _pendingBytes = 0;
  // By normal path as text.
  std::map<std::string, File> _files;
};

}  // namespace xsmp
