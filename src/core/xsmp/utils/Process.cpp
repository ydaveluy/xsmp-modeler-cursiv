#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>
#include <xsmp/utils/FileWriter.hpp>
#include <xsmp/utils/Process.hpp>
#if defined(__wasi__)
// A WASI module cannot start a process, so its host does. A call with a request
// runs the process; a call with room for the answer copies it. Both return the
// size of the answer.
extern "C" __attribute__((import_module("xsmp"),
                          import_name("host_call"))) std::uint32_t
xsmp_host_call(const char*, std::uint32_t, char*, std::uint32_t);
#elif defined(_WIN32)
#include <windows.h>

#include <mutex>
#else
#include <fcntl.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#if defined(__APPLE__)
#include <crt_externs.h>
#endif
#if !defined(__APPLE__)
extern char** environ;
#endif
#endif

namespace xsmp {

namespace {

#if defined(_WIN32)
std::wstring wideText(std::string_view text) {
  if (text.find('\0') != std::string_view::npos)
    throw std::invalid_argument("A process command shall not contain NUL");
  if (text.empty()) return {};
  const int size =
      MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
                          static_cast<int>(text.size()), nullptr, 0);
  if (!size)
    throw std::system_error(GetLastError(), std::system_category(),
                            "Invalid UTF-8 process command");
  std::wstring result(size, L'\0');
  if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
                           static_cast<int>(text.size()), result.data(), size))
    throw std::system_error(GetLastError(), std::system_category(),
                            "Cannot convert process command");
  return result;
}
#endif

#if defined(__wasi__)
// Asks the host to run the process. Numbers are 32-bit little-endian.
//
// The request: the timeout in milliseconds, the output limit, the number of
// command parts, each part as its size and bytes, and the input as its size and
// bytes.
//
// The answer: the failure (0, or 1 + `ProcessError::Kind`), the exit code, the
// size of the output, the size of the errors, the output, and the errors. After
// a failure, the errors hold its message.
ProcessResult askHost(std::span<const std::string> command,
                      std::string_view input, const ProcessOptions& options) {
  std::string request;
  const auto putNumber = [&](std::uint32_t value) {
    request.append(reinterpret_cast<const char*>(&value), sizeof value);
  };
  const auto putText = [&](std::string_view text) {
    putNumber(static_cast<std::uint32_t>(text.size()));
    request += text;
  };
  putNumber(static_cast<std::uint32_t>(options.timeout.count()));
  putNumber(options.maxOutputBytes);
  putNumber(static_cast<std::uint32_t>(command.size()));
  for (const auto& part : command) putText(part);
  putText(input);

  std::string answer(xsmp_host_call(request.data(), request.size(), nullptr, 0),
                     '\0');
  if (xsmp_host_call(nullptr, 0, answer.data(), answer.size()) != answer.size())
    throw std::runtime_error("Incomplete process answer");

  std::array<std::uint32_t, 4> header{};
  if (answer.size() < sizeof header)
    throw std::runtime_error("Invalid process answer");
  std::memcpy(header.data(), answer.data(), sizeof header);
  const auto [failure, exitCode, outputSize, errorsSize] = header;
  const auto body = std::string_view(answer).substr(sizeof header);
  if (body.size() != std::uint64_t{outputSize} + errorsSize || failure > 3)
    throw std::runtime_error("Invalid process answer");

  if (failure)
    throw ProcessError(static_cast<ProcessError::Kind>(failure - 1),
                       std::string(body.substr(outputSize)));
  return {static_cast<int>(exitCode), std::string(body.substr(0, outputSize)),
          std::string(body.substr(outputSize))};
}
#else
// A new temporary directory for the streams of one process, removed with this.
struct ProcessDirectory {
  std::filesystem::path path;

  ProcessDirectory() {
#if !defined(_WIN32)
    auto pattern =
        (std::filesystem::temp_directory_path() / "xsmp-process-XXXXXX")
            .string();
    if (!::mkdtemp(pattern.data()))
      throw std::system_error(errno, std::generic_category(),
                              "Cannot create a temporary process directory");
    path = std::move(pattern);
#else
    const auto stamp =
        std::chrono::steady_clock::now().time_since_epoch().count();
    for (unsigned count = 0;; ++count) {
      path = std::filesystem::temp_directory_path()
             / ("xsmp-process-" + std::to_string(stamp) + "-"
                + std::to_string(count));
      if (std::filesystem::create_directory(path)) return;
    }
#endif
  }

  ~ProcessDirectory() {
    std::error_code error;
    std::filesystem::remove_all(path, error);
  }
};

[[noreturn]] void throwOutputTooLarge() {
  throw ProcessError(ProcessError::Kind::OutputTooLarge,
                     "The process output exceeds its byte limit");
}

// The whole file, which must exist and fit within `maximum`.
std::string readStream(const std::filesystem::path& path,
                       std::uint64_t maximum) {
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  const auto size = file.tellg();
  if (!file || size < 0)
    throw std::runtime_error("Cannot read process output: " + path.string());
  if (static_cast<std::uint64_t>(size) > maximum) throwOutputTooLarge();
  std::string result(static_cast<std::size_t>(size), '\0');
  file.seekg(0);
  file.read(result.data(), static_cast<std::streamsize>(result.size()));
  if (!file)
    throw std::runtime_error("Incomplete process output: " + path.string());
  return result;
}

// Where the streams of a process are.
struct ProcessFiles {
  const std::filesystem::path& input;
  const std::filesystem::path& output;
  const std::filesystem::path& errors;
};

// What `waitFor` calls over and over while the process runs. What it throws
// stops the process.
using WhileRunning = std::function<void()>;

#if defined(_WIN32)
// Starts the command, waits for it, and returns its exit code.
int waitFor(std::span<const std::string> command, const ProcessFiles& files,
            const WhileRunning& whileRunning) {
  // Quotes an argument as CommandLineToArgvW reads it back: backslashes only
  // need doubling before a double quote.
  const auto quote = [](const std::wstring& value) {
    std::wstring out = L"\"";
    unsigned slashes = 0;
    for (const wchar_t character : value) {
      if (character == L'\\') {
        ++slashes;
        continue;
      }
      out.append(slashes * (character == L'"' ? 2 : 1), L'\\');
      slashes = 0;
      if (character == L'"') out += L'\\';
      out += character;
    }
    out.append(slashes * 2, L'\\');
    return out + L'"';
  };

  std::wstring commandLine;
  for (const auto& arg : command) {
    if (!commandLine.empty()) commandLine += L' ';
    commandLine += quote(wideText(arg));
  }

  // A job lets a kill reach what the process starts. The process starts
  // suspended, so it joins the job before it can start anything.
  const HANDLE job = CreateJobObjectW(nullptr, nullptr);
  if (!job)
    throw ProcessError(ProcessError::Kind::CannotStart,
                       std::system_category().message(GetLastError()));

  PROCESS_INFORMATION process{};
  DWORD startError = ERROR_SUCCESS;
  {
    // A process inherits every inheritable handle open when it starts. Those of
    // this process are open only until its own start, or a process started
    // beside it would keep them, and the temporary directory could not be
    // removed.
    static std::mutex startMutex;
    std::scoped_lock lock(startMutex);
    SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    const HANDLE in =
        CreateFileW(files.input.c_str(), GENERIC_READ, FILE_SHARE_READ,
                    &security, OPEN_EXISTING, 0, nullptr);
    if (in == INVALID_HANDLE_VALUE) startError = GetLastError();
    const HANDLE out =
        CreateFileW(files.output.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
                    &security, CREATE_ALWAYS, 0, nullptr);
    if (out == INVALID_HANDLE_VALUE && !startError) startError = GetLastError();
    const HANDLE err =
        CreateFileW(files.errors.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
                    &security, CREATE_ALWAYS, 0, nullptr);
    if (err == INVALID_HANDLE_VALUE && !startError) startError = GetLastError();
    if (!startError) {
      STARTUPINFOW startup{};
      startup.cb = sizeof(startup);
      startup.dwFlags = STARTF_USESTDHANDLES;
      startup.hStdInput = in;
      startup.hStdOutput = out;
      startup.hStdError = err;
      if (!CreateProcessW(nullptr, commandLine.data(), nullptr, nullptr, TRUE,
                          CREATE_SUSPENDED, nullptr, nullptr, &startup,
                          &process))
        startError = GetLastError();
    }
    for (const auto handle : {in, out, err})
      if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle);
  }
  if (startError) {
    CloseHandle(job);
    throw ProcessError(ProcessError::Kind::CannotStart,
                       std::system_category().message(startError));
  }

  auto exitCode = -1;
  try {
    if (!AssignProcessToJobObject(job, process.hProcess)
        || ResumeThread(process.hThread) == static_cast<DWORD>(-1))
      throw ProcessError(ProcessError::Kind::CannotStart,
                         std::system_category().message(GetLastError()));
    for (;;) {
      whileRunning();
      const auto status = WaitForSingleObject(process.hProcess, 10);
      if (status == WAIT_OBJECT_0) break;
      if (status == WAIT_FAILED)
        throw std::system_error(GetLastError(), std::system_category(),
                                "Cannot wait for a process");
    }
    DWORD code = 1;
    if (!GetExitCodeProcess(process.hProcess, &code))
      throw std::system_error(GetLastError(), std::system_category(),
                              "Cannot read a process exit code");
    exitCode = static_cast<int>(code);
  } catch (...) {
    TerminateJobObject(job, 1);
    TerminateProcess(process.hProcess, 1);
    WaitForSingleObject(process.hProcess, INFINITE);
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    CloseHandle(job);
    throw;
  }
  CloseHandle(process.hThread);
  CloseHandle(process.hProcess);
  CloseHandle(job);
  return exitCode;
}
#else
// Starts the command, waits for it, and returns its exit code.
int waitFor(std::span<const std::string> command, const ProcessFiles& files,
            const WhileRunning& whileRunning) {
  std::vector<char*> argv;
  for (const auto& arg : command)
    argv.push_back(const_cast<char*>(arg.c_str()));
  argv.push_back(nullptr);

  // Independent close-on-exec sources for posix_spawn's stdio actions.
  const auto openStream = [](const std::filesystem::path& path, int flags) {
    const auto original = ::open(path.c_str(), flags | O_CLOEXEC, 0600);
    if (original < 0 || original > STDERR_FILENO) return original;
    // Closed parent stdio must not let a subsequent dup2 overwrite another
    // source descriptor. Keep every source above 2 and close-on-exec.
    const auto moved = ::fcntl(original, F_DUPFD_CLOEXEC, STDERR_FILENO + 1);
    const auto error = errno;
    ::close(original);
    errno = error;
    return moved;
  };

  const auto in = openStream(files.input, O_RDONLY);
  const auto out = openStream(files.output, O_WRONLY | O_CREAT | O_TRUNC);
  const auto err = openStream(files.errors, O_WRONLY | O_CREAT | O_TRUNC);
  if (in < 0 || out < 0 || err < 0) {
    if (in >= 0) ::close(in);
    if (out >= 0) ::close(out);
    if (err >= 0) ::close(err);
    throw ProcessError(ProcessError::Kind::CannotStart,
                       "Cannot prepare the process streams");
  }

  posix_spawn_file_actions_t actions;
  posix_spawnattr_t attributes;
  auto spawnError = ::posix_spawn_file_actions_init(&actions);
  if (spawnError) {
    ::close(in);
    ::close(out);
    ::close(err);
    throw ProcessError(ProcessError::Kind::CannotStart,
                       std::generic_category().message(spawnError));
  }
  spawnError = ::posix_spawnattr_init(&attributes);
  if (spawnError) {
    ::posix_spawn_file_actions_destroy(&actions);
    ::close(in);
    ::close(out);
    ::close(err);
    throw ProcessError(ProcessError::Kind::CannotStart,
                       std::generic_category().message(spawnError));
  }

  const auto configure = [&] {
    for (const auto [source, target] :
         {std::pair{in, STDIN_FILENO}, std::pair{out, STDOUT_FILENO},
          std::pair{err, STDERR_FILENO}}) {
      if (const auto error =
              ::posix_spawn_file_actions_adddup2(&actions, source, target))
        return error;
      if (const auto error =
              ::posix_spawn_file_actions_addclose(&actions, source))
        return error;
    }
    // A process group of its own, so a kill can reach what the process starts.
    if (const auto error =
            ::posix_spawnattr_setflags(&attributes, POSIX_SPAWN_SETPGROUP))
      return error;
    return ::posix_spawnattr_setpgroup(&attributes, 0);
  };

  spawnError = configure();
  auto child = -1;
  // The `p` form finds a bare name through PATH.
  if (!spawnError)
    spawnError =
        ::posix_spawnp(&child, argv.front(), &actions, &attributes, argv.data(),
#if defined(__APPLE__)
                       *_NSGetEnviron());
#else
                       environ);
#endif
  ::posix_spawn_file_actions_destroy(&actions);
  ::posix_spawnattr_destroy(&attributes);
  ::close(in);
  ::close(out);
  ::close(err);
  if (spawnError)
    throw ProcessError(ProcessError::Kind::CannotStart,
                       std::generic_category().message(spawnError));

  auto status = 0;
  try {
    // Short pauses first, as a small tool ends within milliseconds.
    auto pause = std::chrono::milliseconds(1);
    for (;;) {
      whileRunning();
      const auto waited = ::waitpid(child, &status, WNOHANG);
      if (waited == child) break;
      if (waited < 0 && errno != EINTR)
        throw std::system_error(errno, std::generic_category(),
                                "Cannot wait for a process");
      std::this_thread::sleep_for(pause);
      pause = std::min(pause * 2, std::chrono::milliseconds(10));
    }
  } catch (...) {
    // Kill the child's group, including subprocesses inheriting its streams,
    // and always reap our direct child before propagating the error.
    ::kill(-child, SIGTERM);
    ::kill(child, SIGTERM);
    auto reaped = false;
    const auto grace =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(100);
    while (std::chrono::steady_clock::now() < grace) {
      if (::waitpid(child, &status, WNOHANG) == child) {
        reaped = true;
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    ::kill(-child, SIGKILL);
    if (!reaped) {
      ::kill(child, SIGKILL);
      while (::waitpid(child, &status, 0) < 0 && errno == EINTR) {
      }
    }
    throw;
  }
  return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}
#endif
#endif

}  // namespace

ProcessResult runProcess(std::span<const std::string> command,
                         std::string_view input,
                         const ProcessOptions& options) {
  if (command.empty()) throw std::invalid_argument("A process needs a command");
#if defined(__wasi__)
  return askHost(command, input, options);
#else
  // The streams are files, not pipes, so nothing has to drain them. This thread
  // polls instead of blocking, to notice the deadline and output over the limit
  // while the process runs.
  ProcessDirectory directory;
  const auto inputFile = directory.path / "input";
  const auto outputFile = directory.path / "output";
  const auto errorsFile = directory.path / "errors";
  (void)writeFileIfChanged(inputFile, input);

  const auto started = std::chrono::steady_clock::now();
  const auto checkOutput = [&] {
    for (const auto& path : {outputFile, errorsFile}) {
      std::error_code error;
      const auto size = std::filesystem::file_size(path, error);
      if (!error && size > options.maxOutputBytes) throwOutputTooLarge();
    }
  };
  const auto whileRunning = [&] {
      if (options.timeout.count()
        && std::chrono::steady_clock::now() - started >= options.timeout)
      throw ProcessError(ProcessError::Kind::TimedOut,
                         "The process timed out after "
                             + std::to_string(options.timeout.count()) + " ms");
    checkOutput();
  };

  ProcessResult result;
  result.exitCode =
      waitFor(command, {inputFile, outputFile, errorsFile}, whileRunning);
  checkOutput();
  result.output = readStream(outputFile, options.maxOutputBytes);
  // A failure to read the errors must not hide the result.
  try {
    result.errors = readStream(errorsFile, options.maxOutputBytes);
  } catch (...) {
    result.errors.clear();
  }
  return result;
#endif
}

std::string environmentVariable(const char* name) {
#if defined(_WIN32)
  const auto key = wideText(name);
  const auto* value = _wgetenv(key.c_str());
  if (!value || !*value) return {};
  const int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value, -1,
                                       nullptr, 0, nullptr, nullptr);
  if (!size)
    throw std::system_error(GetLastError(), std::system_category(),
                            "Invalid environment variable");
  std::string result(size, '\0');
  WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value, -1, result.data(),
                      size, nullptr, nullptr);
  result.pop_back();
  return result;
#else
  const auto* value = std::getenv(name);
  return value ? value : "";
#endif
}

}  // namespace xsmp
