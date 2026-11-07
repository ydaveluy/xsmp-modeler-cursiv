#include <algorithm>
#include <array>
#include <cctype>
#include <cursiv/services/SharedCoreServices.hpp>
#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <cursiv/workspace/FileSystemProvider.hpp>
#include <utility>
#include <xsmp/workspace/SourceUri.hpp>

namespace xsmp {

std::string_view dirnameOfUri(std::string_view uri) noexcept {
  const auto slash = uri.find_last_of('/');
  return slash == std::string_view::npos ? uri : uri.substr(0, slash);
}

std::string resolveSourceUri(std::string_view projectDirectoryUri,
                             std::string_view relativePath) {
  std::string uri(projectDirectoryUri);
  const auto authorityEnd = uri.find("://");
  const auto root = authorityEnd == std::string::npos
                        ? std::size_t{0}
                        : uri.find('/', authorityEnd + 3);
  // A source path is a path, so `.` and `..` are resolved here: a URI keeps
  // them as written.
  const auto addSegment = [&](std::string_view segment) {
    if (segment.empty() || segment == ".") return;
    if (segment == "..") {
      const auto parent = uri.rfind('/');
      if (parent != std::string::npos && parent > root) uri.resize(parent);
      return;
    }
    uri += '/';
    for (const auto character : segment) {
      switch (character) {
        case '#':
          uri += "%23";
          break;
        case '?':
          uri += "%3F";
          break;
        case '%':
          uri += "%25";
          break;
        default:
          uri += character;
          break;
      }
    }
  };
  std::size_t start = 0;
  for (std::size_t i = 0; i <= relativePath.size(); ++i) {
    if (i < relativePath.size() && relativePath[i] != '/'
        && relativePath[i] != '\\')
      continue;
    addSegment(relativePath.substr(start, i - start));
    start = i + 1;
  }
  return cursiv::normalize_uri(uri);
}

bool isSmpMirrorUri(std::string_view uri) noexcept {
  return uri.starts_with("xsmp-smp:");
}

namespace {

// `uri` with its extension, which is `from`, replaced by `to`.
std::string withExtension(std::string uri, std::string_view from,
                          std::string_view to) {
  const auto suffix = uri.find_first_of("?#");
  const auto pathEnd = suffix == std::string::npos ? uri.size() : suffix;
  return uri.replace(pathEnd - from.size(), from.size(), to);
}

}  // namespace

std::string_view smpMirrorNativeExtension(std::string_view uri) {
  if (!isSmpMirrorUri(uri)) return {};
  const auto extension = cursiv::uri_extension(uri);
  for (const auto& [source, native] : smpExtensions)
    if (extension == source) return native;
  return {};
}

std::string nativeFileName(std::string_view uri) {
  auto name = std::string(cursiv::uri_file_name(uri));
  if (const auto native = smpMirrorNativeExtension(uri); !native.empty())
    name.replace(name.size() - cursiv::uri_extension(uri).size(),
                 std::string::npos, native);
  return name;
}

bool hasNativeSidecar(std::string_view uri,
                      const cursiv::SharedCoreServices& shared) {
  const auto native = smpMirrorNativeExtension(uri);
  if (native.empty()) return false;
  const auto nativeUri = cursiv::normalize_uri(
      withExtension("file" + std::string(uri.substr(uri.find(':'))),
                    cursiv::uri_extension(uri), native));
  if (shared.documents && shared.documents->hasDocument(nativeUri)) return true;
  return shared.fileSystemProvider
         && shared.fileSystemProvider->exists(nativeUri);
}

bool isReadOnlyDocumentUri(std::string_view uri) noexcept {
  return isSmpMirrorUri(uri) || isBuiltinUri(uri);
}

bool isBuiltinUri(std::string_view uri) noexcept {
  return uri.starts_with("xsmp:");
}

std::string canonicalVirtualUri(std::string_view uri) {
  const auto colon = uri.find(':');
  if (colon == std::string_view::npos) return std::string(uri);
  const auto rest = uri.substr(colon);
  const auto suffix = std::min(rest.find_first_of("?#"), rest.size());
  const auto path =
      cursiv::file_uri_to_path("file" + std::string(rest.substr(0, suffix)));
  if (!path) return std::string(uri);
  // The way the protocol library spells a path, so that a URI a client sends
  // back is the one the document was stored under.
  constexpr std::string_view hex = "0123456789ABCDEF";
  std::string canonical(uri.substr(0, colon + 1));
  for (const unsigned char c : *path) {
    if (std::isalnum(c) || c == '_' || c == '.' || c == '-' || c == '/') {
      canonical += static_cast<char>(c);
    } else {
      canonical += '%';
      canonical += hex[c >> 4];
      canonical += hex[c & 15];
    }
  }
  return canonical.append(rest.substr(suffix));
}

std::string sourceDocumentUri(std::string_view uri) {
  if (!isSmpMirrorUri(uri)) return cursiv::normalize_uri(uri);
  auto original = "file" + std::string(uri.substr(uri.find(':')));
  // A mirror URI of an earlier version ends with the native extension.
  const auto extension = cursiv::uri_extension(uri);
  for (const auto& [source, native] : smpExtensions)
    if (extension == native) {
      original = withExtension(std::move(original), native, source);
      break;
    }
  return cursiv::normalize_uri(original);
}

std::string smpMirrorUri(std::string_view sourceUri) {
  const auto uri = cursiv::normalize_uri(sourceUri);
  const auto extension = cursiv::uri_extension(uri);
  for (const auto& [source, native] : smpExtensions)
    if (extension == source)
      return canonicalVirtualUri("xsmp-smp" + uri.substr(uri.find(':')));
  return {};
}

}  // namespace xsmp
