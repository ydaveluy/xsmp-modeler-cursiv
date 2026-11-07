#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <map>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <xsmp/Extension.hpp>
#include <xsmp/utils/Process.hpp>

#if defined(__wasi__)
// The WASI host links a library into this program.
extern "C" {
__attribute__((import_module("xsmp"), import_name("load_library")))
std::uint32_t xsmp_load_library(const char*, std::uint32_t);
__attribute__((import_module("xsmp"), import_name("load_symbol")))
void* xsmp_load_symbol(std::uint32_t, const char*, std::uint32_t);
__attribute__((import_module("xsmp"), import_name("load_error")))
std::uint32_t xsmp_load_error(char*, std::uint32_t);

// A library reads `errno` through this: see `wasi-errno.h`.
int* xsmp_errno_location() { return &errno; }
}
#elif !defined(_WIN32)
#include <dlfcn.h>
#endif

namespace xsmp {

namespace {

struct LoadedLibrary {
  ExtensionRegistration registration = nullptr;
  std::string failure;
};

std::mutex librariesMutex;
std::map<std::string, LoadedLibrary> libraries;

std::string libraryKey(const std::filesystem::path& path) {
  return std::filesystem::absolute(path).lexically_normal().generic_string();
}

#if defined(_WIN32)
void* openLibrary(const std::string&) {
  throw std::runtime_error(
      "A native Windows server loads no extension: use the WASI server");
}
void* librarySymbol(void*, const char*) { return nullptr; }
#elif defined(__wasi__)
[[noreturn]] void throwLoadError() {
  std::string message(4096, '\0');
  message.resize(std::min<std::size_t>(
      xsmp_load_error(message.data(),
                      static_cast<std::uint32_t>(message.size())),
      message.size()));
  throw std::runtime_error(message);
}
void* openLibrary(const std::string& path) {
  const auto library = xsmp_load_library(
      path.data(), static_cast<std::uint32_t>(path.size()));
  if (!library) throwLoadError();
  return reinterpret_cast<void*>(static_cast<std::uintptr_t>(library));
}
void* librarySymbol(void* library, const char* name) {
  auto* const symbol =
      xsmp_load_symbol(static_cast<std::uint32_t>(
                           reinterpret_cast<std::uintptr_t>(library)),
                       name, static_cast<std::uint32_t>(std::strlen(name)));
  if (!symbol) throwLoadError();
  return symbol;
}
#else
void* openLibrary(const std::string& path) {
  if (path.ends_with(".wasm"))
    throw std::runtime_error(
        "A native server loads a native library, and this one is for a WASI "
        "server: "
        + path);
  auto* const library = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
  if (!library) throw std::runtime_error(dlerror());
  return library;
}
void* librarySymbol(void* library, const char* name) {
  auto* const symbol = dlsym(library, name);
  if (!symbol)
    throw std::runtime_error(std::string("This library has no function ")
                             + name);
  return symbol;
}
#endif

LoadedLibrary load(const std::string& path) {
  LoadedLibrary loaded;
  try {
    auto* const library = openLibrary(path);
    const auto abi = reinterpret_cast<const char* (*)()>(
        librarySymbol(library, "xsmp_extension_abi"))();
    if (abi != extensionAbi)
      throw std::runtime_error("This library was built for '" + std::string(abi)
                               + "', and this server is '"
                               + std::string(extensionAbi) + "'");
    loaded.registration = reinterpret_cast<ExtensionRegistration>(
        librarySymbol(library, "xsmp_register"));
  } catch (const std::exception& error) {
    loaded.failure = error.what();
  }
  return loaded;
}

}  // namespace

void loadExtensionLibraries(std::span<const std::string> packages) {
  std::vector<std::filesystem::path> paths = packageLibraries(packages);
  std::istringstream named(environmentVariable("XSMP_EXTENSION_LIBRARIES"));
  for (std::string line; std::getline(named, line);)
    if (!line.empty()) paths.emplace_back(line);
  const std::scoped_lock lock(librariesMutex);
  for (const auto& path : paths) {
    const auto key = libraryKey(path);
    if (!libraries.contains(key)) libraries.emplace(key, load(key));
  }
}

ExtensionRegistration extensionRegistration(const std::filesystem::path& path) {
  const auto key = libraryKey(path);
  const std::scoped_lock lock(librariesMutex);
  auto found = libraries.find(key);
  if (found == libraries.end()) {
#if defined(__wasi__)
    // A library loads before the first thread, and this server has threads.
    throw std::runtime_error(
        "This extension came after the server started: restart the server");
#else
    found = libraries.emplace(key, load(key)).first;
#endif
  }
  if (!found->second.registration)
    throw std::runtime_error(found->second.failure);
  return found->second.registration;
}

}  // namespace xsmp
