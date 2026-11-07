#pragma once

#include <chrono>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

namespace xsmp {

struct ProcessOptions {
  /// How long the process may run. Zero means no limit.
  std::chrono::milliseconds timeout{0};
  /// The most bytes each of its output and error streams may hold.
  std::uint32_t maxOutputBytes = 256u * 1024u * 1024u;
};

struct ProcessResult {
  /// The exit code, or -1 when a signal ended the process.
  int exitCode = -1;
  std::string output;
  std::string errors;
};

/// Why a process did not run to its end. Its message says no more than that, so
/// a caller can word it for its own users.
class ProcessError : public std::runtime_error {
 public:
  enum class Kind { CannotStart, TimedOut, OutputTooLarge };

  ProcessError(Kind kind, const std::string& message)
      : std::runtime_error(message), _kind(kind) {}

  [[nodiscard]] Kind kind() const noexcept { return _kind; }

 private:
  Kind _kind;
};

/// Runs `command` to its end, with `input` on its standard input. Its first
/// part is the program, which a bare name finds through PATH. Stopping the
/// process stops what it started too. A WASI module asks its host to do all
/// this.
///
/// Throws `ProcessError` for what the process did.
[[nodiscard]] ProcessResult runProcess(std::span<const std::string> command,
                                       std::string_view input,
                                       const ProcessOptions& options = {});

/// An environment variable as UTF-8, empty when it is not set.
[[nodiscard]] std::string environmentVariable(const char* name);

}  // namespace xsmp
