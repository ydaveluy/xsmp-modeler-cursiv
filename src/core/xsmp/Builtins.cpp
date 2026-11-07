#include <cursiv/services/SharedCoreServices.hpp>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/DocumentBuilder.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <string>
#include <vector>
#include <xsmp/Builtins.hpp>

namespace xsmp {

void loadXsmpcatBuiltins(cursiv::SharedCoreServices& shared) {
  const auto sources = builtinSources();
  std::vector<cursiv::Document*> documents;
  documents.reserve(sources.size());
  for (const auto& source : sources) {
    if (!shared.documents->hasDocument(source.uri))
      documents.push_back(shared.documents
                              ->createDocument(std::string(source.uri),
                                               std::string(source.text))
                              .get());
  }

  cursiv::BuildOptions options;
  options.validation = false;  // builtins are trusted; index + link only
  shared.documentBuilder->build(documents, options);
}

}  // namespace xsmp
