#include <string>
#include <xsmp/workspace/SourceUri.hpp>
#include <xsmp/workspace/XsmpServiceRegistry.hpp>

namespace xsmp {

const cursiv::CoreServices& XsmpServiceRegistry::getServices(
    std::string_view uri, std::string_view languageId) const {
  if (const auto* services = findServices(uri, languageId)) return *services;
  if (isSmpMirrorUri(uri))
    throw cursiv::ServiceRegistryError("Unsupported SMP mirror URI '"
                                       + std::string(uri) + "'.");
  return DefaultServiceRegistry::getServices(uri, languageId);
}

const cursiv::CoreServices* XsmpServiceRegistry::findServices(
    std::string_view uri, std::string_view languageId) const {
  if (!isSmpMirrorUri(uri))
    return DefaultServiceRegistry::findServices(uri, languageId);
  // A client may name the language of the XML file it read: the kind of the
  // file decides.
  const auto native = smpMirrorNativeExtension(uri);
  return native.empty()
             ? nullptr
             : DefaultServiceRegistry::findServices(uri, native.substr(1));
}

}  // namespace xsmp
