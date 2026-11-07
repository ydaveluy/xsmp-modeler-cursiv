#pragma once

#include <cursiv/cli.hpp>
#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/BuildOptions.hpp>
#include <fstream>
#include <sstream>
#include <xsmp/utils/TreeUtils.hpp>

#include "support/LspTestSupport.hpp"

namespace xsmp {

template <class Documents>
void buildDocuments(cursiv::DocumentBuilder& builder,
                    const Documents& documents,
                    const cursiv::BuildOptions& options = {}) {
  std::vector<cursiv::Document*> raw;
  for (const auto& document : documents) raw.push_back(document.get());
  builder.build(raw, options);
}

inline std::shared_ptr<cursiv::Document> parsePath(
    std::string_view path, cursiv::CoreServices& language,
    bool validation = true) {
  std::ifstream file{std::string(path)};
  if (!file)
    throw std::runtime_error("Cannot read test fixture " + std::string(path));
  std::ostringstream text;
  text << file.rdbuf();
  const auto uri = cursiv::path_to_file_uri(std::string(path));
  auto document = language.shared().documents->createDocument(uri, text.str());
  auto* raw = document.get();
  language.shared().documentBuilder->build(std::span(&raw, 1),
                                           {.validation = validation});
  return document;
}

}  // namespace xsmp
