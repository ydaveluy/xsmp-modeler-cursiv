#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

namespace xsmp {

inline std::filesystem::path makeScratchDirectory(std::string_view prefix) {
  const auto stamp =
      std::chrono::steady_clock::now().time_since_epoch().count();
  const auto base = std::filesystem::temp_directory_path()
                    / (std::string(prefix) + std::to_string(stamp));
  // WASI processes reuse the same linear-memory addresses. Reserve the path
  // atomically instead of using an object's address as a process identifier.
  for (std::size_t attempt = 0;; ++attempt) {
    auto candidate = base;
    candidate += "-" + std::to_string(attempt);
    if (std::filesystem::create_directory(candidate)) return candidate;
  }
}

// A fresh directory under the system temp folder, removed on destruction.
struct ScratchDirectory {
  std::filesystem::path path = makeScratchDirectory("xsmp-test-");

  ScratchDirectory() = default;
  ScratchDirectory(const ScratchDirectory&) = delete;
  ScratchDirectory& operator=(const ScratchDirectory&) = delete;

  ~ScratchDirectory() {
    std::error_code ignored;
    std::filesystem::remove_all(path, ignored);
  }

  std::filesystem::path write(std::string_view name,
                              std::string_view content) const {
    auto file = path / name;
    std::filesystem::create_directories(file.parent_path());
    std::ofstream stream(file, std::ios::binary);
    stream << content;
    return file;
  }
};

inline std::string readFile(const std::filesystem::path& path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) throw std::runtime_error("Cannot read " + path.string());
  std::ostringstream text;
  text << file.rdbuf();
  return text.str();
}

inline std::string readFixture(std::string_view name) {
  return readFile(std::filesystem::path(XSMP_FIXTURES_DIR) / name);
}

}  // namespace xsmp
