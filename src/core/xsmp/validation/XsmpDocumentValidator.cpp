#include "XsmpDocumentValidator.hpp"

#include <algorithm>
#include <cursiv/references/ScopeProvider.hpp>
#include <cursiv/services/CoreServices.hpp>
#include <cursiv/syntax-tree/AstReflection.hpp>
#include <cursiv/syntax-tree/DocumentReferences.hpp>
#include <cursiv/utils/UriUtils.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <cursiv/workspace/IndexManager.hpp>
#include <optional>
#include <span>
#include <vector>
#include <xsmp/LiteralConversions.hpp>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/utils/TreeUtils.hpp>
#include <xsmp/utils/Utf8.hpp>
#include <xsmp/workspace/SourceUri.hpp>

namespace xsmp {

namespace {

// Calls `visit` with each string and character literal of `document`, in order,
// with its text as written.
void forEachLiteral(const cursiv::Document& document, const auto& visit) {
  const auto& cst = document.parseResult.cst();
  const auto leaves = cst.leaf_count();
  for (cursiv::NodeCount rank = 0; rank < leaves; ++rank) {
    const auto leaf = cst.leaf(rank);
    if (leaf.hidden) continue;
    const auto text = cst.text(leaf.id);
    if (text.starts_with('"') || text.starts_with('\'')) visit(leaf, text);
  }
}

}  // namespace

void checkEncoding(const Document& root, cursiv::ValidationAcceptor& accept) {
  const auto* document = root.getDocument();
  if (!document) return;
  const auto text = document->textDocument().getText();
  if (utf8Length(text) == text.size()) return;

  // The first bad byte outside the literals: the parser reports those inside.
  std::optional<std::size_t> bad;
  std::size_t gap = 0;
  const auto checkGap = [&](std::size_t end) {
    if (const auto length = utf8Length(text.substr(gap, end - gap));
        !bad && length < end - gap)
      bad = gap + length;
  };
  forEachLiteral(*document, [&](const cursiv::CstLeaf& leaf, std::string_view) {
    checkGap(leaf.begin);
    gap = leaf.end;
  });
  checkGap(text.size());
  if (bad)
    accept.warning(
        cursiv::TextRange{.begin = static_cast<cursiv::TextOffset>(*bad),
                          .end = static_cast<cursiv::TextOffset>(*bad + 1)},
        "This file is not valid UTF-8.");
}

void checkEscapes(const Document& root, cursiv::ValidationAcceptor& accept) {
  const auto* document = root.getDocument();
  if (!document
      || document->textDocument().getText().find('\\')
             == std::string_view::npos)
    return;
  forEachLiteral(*document, [&](const cursiv::CstLeaf& leaf,
                                std::string_view literal) {
    for (auto at = literal.find('\\'); at != std::string_view::npos;
         at = literal.find('\\', at + 2)) {
      // A byte that is not UTF-8 has its own error: see `quoted_text`.
      const auto size = utf8CharacterSize(literal.substr(at + 1));
      if (size == 0 || isKnownEscape(literal[at + 1])) continue;
      const auto begin = leaf.begin + static_cast<cursiv::TextOffset>(at);
      accept.warning(
          cursiv::TextRange{
              .begin = begin,
              .end = begin + static_cast<cursiv::TextOffset>(size + 1)},
          "Unknown escape '" + std::string(literal.substr(at, size + 1)) + "'");
    }
  });
}

void checkOutputFile(const Document& root, cursiv::ValidationAcceptor& accept) {
  const auto* document = root.getDocument();
  if (!document) return;
  const auto* shared = dynamic_cast<const XsmpSharedCoreServices*>(
      &document->language->shared());
  if (!shared || !shared->projectManager) return;
  const auto* project = shared->projectManager->getProject(*document);
  const auto writesSmp =
      project
      && std::ranges::any_of(project->elements, [](const auto* element) {
           const auto* tool = cursiv::ast_cast<ToolReference>(element);
           return tool && tool->tool.refText == "smp";
         });
  if (!writesSmp) return;

  // A mirror that a native file shadows writes nothing.
  if (hasNativeSidecar(document->uri, *shared)) return;
  std::vector<cursiv::DiagnosticRelatedInformation> related;
  // A mirror writes as the native file of its kind would.
  const auto name = nativeFileName(document->uri);
  for (const auto& other : shared->documents->all()) {
    const auto* otherRoot =
        cursiv::ast_cast<const Document>(other->parseResult.value);
    if (other.get() != document && otherRoot
        && nativeFileName(other->uri) == name
        && shared->projectManager->getProject(*other) == project
        && !hasNativeSidecar(other->uri, *shared))
      related.push_back({.uri = other->uri,
                         .message = std::string(name),
                         .range = rangeForFeature(*otherRoot, "name")});
  }
  if (!related.empty())
    accept
        .error(root,
               "Another document of this project has this file name, and SMP "
               "generation would write both to the same file.")
        .range(rangeForFeature(root, "name"))
        .relatedInformation(
            std::span<const cursiv::DiagnosticRelatedInformation>(related));
}

void XsmpDocumentValidator::processLinkingErrors(
    const cursiv::Document& document,
    std::vector<cursiv::Diagnostic>& diagnostics,
    const cursiv::CancellationToken& token) const {
  // The base class writes one diagnostic per unresolved reference, in the order
  // of the walk below, so `at` pairs each reference with its diagnostic.
  auto at = diagnostics.size();
  DefaultDocumentValidator::processLinkingErrors(document, diagnostics, token);
  const auto* references = document.parseResult.references.get();
  if (!references) return;

  references->visitUnresolved(
      document.parseResult.cst(),
      [&](cursiv::RefState why, cursiv::TextOffset, cursiv::TextOffset,
          const cursiv::ReferenceInfo& site) {
        cursiv::throw_if_cancelled(token);
        auto& diagnostic = diagnostics.at(at++);
        // A name that exists but is not of the kind this site accepts.
        const auto* wanted = core().astReflection->referenceNarrowTarget(
            site.reference, site.narrow);
        if (wanted && why == cursiv::RefState::ErrorNotFound) {
          auto broad = site;
          broad.narrow = cursiv::kNoNarrow;
          if (const auto* found = core().scopeProvider->getScopeEntry(
                  broad, site.referenceText)) {
            diagnostic.message = "'" + std::string(site.referenceText) + "' ("
                                 + std::string(found->type->name)
                                 + ") cannot be used here: expected "
                                 + std::string(wanted->name) + ".";
            diagnostic.data.reset();
            return;
          }
        }

        // A name that only misses a dependency of the project.
        if (const auto* shared =
                dynamic_cast<const XsmpSharedCoreServices*>(&core().shared());
            shared && shared->projectManager
            && why == cursiv::RefState::ErrorNotFound) {
          const Project* project = nullptr;
          (void)shared->indexManager->visitByName(
              site.referenceText, true, {}, *core().astReflection,
              [&](const cursiv::AstNodeDescription& candidate) {
                if (candidate.node)
                  if (const auto* home = candidate.node->getDocument())
                    project = shared->projectManager->getProject(*home);
                return !project;
              });
          if (project)
            diagnostic.message += " It is defined in project '"
                                  + std::string(project->name)
                                  + "': add it as a dependency.";
        }
      });
}

}  // namespace xsmp
