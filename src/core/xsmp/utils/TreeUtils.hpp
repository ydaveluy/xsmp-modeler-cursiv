#pragma once

#include <algorithm>
#include <cursiv/services/CoreServices.hpp>
#include <cursiv/syntax-tree/AstReflection.hpp>
#include <cursiv/syntax-tree/AstWalk.hpp>
#include <cursiv/syntax-tree/CstUtils.hpp>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <cursiv/workspace/IndexManager.hpp>
#include <stdexcept>
#include <vector>

// Small tree/index adapters shared by the handwritten language services.
namespace xsmp {

// The document of `node`. Throws when it has none.
inline const cursiv::Document& documentOf(const cursiv::AstNode& node) {
  if (const auto* document = node.getDocument()) return *document;
  throw std::invalid_argument("The XSMP node is not attached to a document");
}

// Every node below `node`, parents first. Empty when it has no document.
inline std::vector<const cursiv::AstNode*> descendantsOf(
    const cursiv::AstNode& node) {
  std::vector<const cursiv::AstNode*> result;
  if (const auto* document = node.getDocument())
    cursiv::visit_all_contents(
        *document->language->astReflection, node,
        [&](const cursiv::AstNode& child) { result.push_back(&child); });
  return result;
}

// The exported symbols of `type` (all types when null) from `documents` (all
// documents when empty), as a list.
inline std::vector<cursiv::AstNodeDescription> indexedElements(
    const cursiv::IndexManager& index, const cursiv::AstTypeTag* type,
    std::span<const cursiv::DocumentId> documents = {}) {
  std::vector<cursiv::AstNodeDescription> result;
  (void)index.visitElements(type, documents,
                            [&](const cursiv::AstNodeDescription& entry) {
                              result.push_back(entry);
                              return true;
                            });
  return result;
}

// The current node of an index entry. Throws when it or its document is gone.
inline const cursiv::AstNode& resolveNode(
    const cursiv::Documents& documents,
    const cursiv::AstNodeDescription& description) {
  // Present while the entry's own build stands, and the index drops the entries
  // of a document it rebuilds.
  if (description.node) return *description.node;
  const auto document = documents.getDocument(description.documentId);
  if (!document)
    throw std::invalid_argument("The XSMP symbol document is absent");
  for (auto* node : document->parseResult.nodes())
    if (node->cstNodeId == description.symbolId
        && node->type == description.type)
      return *node;
  throw std::invalid_argument("The XSMP symbol is absent from its document");
}

// Where `feature` of `node` is written, or the whole node when it is not. Empty
// when the node has no text.
inline cursiv::TextRange rangeForFeature(const cursiv::AstNode& node,
                                         std::string_view feature,
                                         std::size_t index = 0) {
  const auto* document = node.getDocument();
  if (!document || node.cstNodeId == cursiv::kNoNode) return {};
  const cursiv::CstNodeView view{.cst = &document->parseResult.cst(),
                                 .id = node.cstNodeId};
  if (const auto field =
          cursiv::find_node_for_feature(node, feature, index, document))
    return {.begin = field->begin(), .end = field->end()};
  return {.begin = view.begin(), .end = view.end()};
}

// Whether the parser has reported something in the text of `node`, or of its
// `feature` when one is named and written: a literal that did not decode, or
// one it repaired. Nothing more is to be said of its value.
inline bool holdsReportedLiteral(const cursiv::AstNode& node,
                                 std::string_view feature = {}) {
  const auto* document = node.getDocument();
  if (!document || node.cstNodeId == cursiv::kNoNode
      || document->parseResult.diagnostics.empty())
    return false;
  const cursiv::CstNodeView view{.cst = &document->parseResult.cst(),
                                 .id = node.cstNodeId};
  auto begin = view.begin();
  auto end = view.end();
  if (!feature.empty()) {
    const auto written =
        cursiv::find_node_for_feature(node, feature, 0, document);
    if (!written) return false;
    begin = written->begin();
    end = written->end();
  }
  return std::ranges::any_of(
      document->parseResult.diagnostics, [&](const auto& diagnostic) {
        return diagnostic.begin >= begin && diagnostic.end <= end;
      });
}

}  // namespace xsmp
