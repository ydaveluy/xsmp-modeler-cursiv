#pragma once

#include <array>
#include <string>
#include <string_view>
#include <utility>

namespace cursiv {

struct SharedCoreServices;
}

namespace xsmp {

// Each SMP XML extension, and the native extension of the same kind of
// document.
inline constexpr std::array<std::pair<std::string_view, std::string_view>, 5>
    smpExtensions{{{".smpcat", ".xsmpcat"},
                   {".smpasb", ".xsmpasb"},
                   {".smpcfg", ".xsmpcfg"},
                   {".smplnk", ".xsmplnk"},
                   {".smpsed", ".xsmpsed"}}};

// XML-derived models have their own immutable editor document. Their project
// and dependency ownership is the ownership of the physical XML source.
[[nodiscard]] bool isSmpMirrorUri(std::string_view uri) noexcept;

// The extension of the native document that the mirror `uri` stands for:
// `.xsmpcat` for `xsmp-smp:/a/b.smpcat`. Empty for any other URI, and for the
// mirror URIs of earlier versions, which ended with that native extension.
[[nodiscard]] std::string_view smpMirrorNativeExtension(std::string_view uri);

// The file name of the native document that `uri` stands for: `b.xsmpcat` for
// the mirror `xsmp-smp:/a/b.smpcat`, the file name of `uri` otherwise.
[[nodiscard]] std::string nativeFileName(std::string_view uri);

// Whether a native XSMP file stands beside the XML source of this mirror, open
// in the workspace or on disk. False for any other URI.
[[nodiscard]] bool hasNativeSidecar(std::string_view mirrorUri,
                                    const cursiv::SharedCoreServices& shared);

// A document the user cannot edit: a mirror or a built-in.
[[nodiscard]] bool isReadOnlyDocumentUri(std::string_view uri) noexcept;

// A document of the embedded or contributed libraries.
[[nodiscard]] bool isBuiltinUri(std::string_view uri) noexcept;

// The one spelling of a URI of a virtual scheme, which is what a client sends
// back: no empty authority, and the path encoded the way the protocol does.
[[nodiscard]] std::string canonicalVirtualUri(std::string_view uri);

// The physical file behind `uri`: the XML source for a mirror, otherwise `uri`
// itself, normalized.
[[nodiscard]] std::string sourceDocumentUri(std::string_view uri);

// The mirror of an SMP XML file: the same path in the `xsmp-smp` scheme, so
// that an editor shows the name of the XML file. Empty for any other extension.
[[nodiscard]] std::string smpMirrorUri(std::string_view sourceUri);

// The directory URI containing `uri`: everything before its last '/'.
[[nodiscard]] std::string_view dirnameOfUri(std::string_view uri) noexcept;

// Resolve the descriptor's filesystem path without interpreting literal #, ?
// or percent escapes as URI syntax. Backslashes are portable separators.
[[nodiscard]] std::string resolveSourceUri(std::string_view projectDirectoryUri,
                                           std::string_view relativePath);

}  // namespace xsmp
