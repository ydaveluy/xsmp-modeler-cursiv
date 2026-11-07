#include <algorithm>
#include <cursiv/references/References.hpp>
#include <cursiv/services/SharedCoreServices.hpp>
#include <cursiv/text/LspPosition.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <string>
#include <xsmp/XsmpRenameProvider.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/workspace/SourceUri.hpp>

namespace xsmp {

namespace {

// The `.`-joined name of `node` once `renamed` is called `newName`, up to the
// root of its file. Empty for an unnamed node.
std::string qualifiedAfterRename(const cursiv::AstNode& node,
                                 const cursiv::AstNode& renamed,
                                 std::string_view newName) {
  const auto* named = cursiv::ast_cast<NamedElement>(&node);
  if (!named) return {};
  std::string name(&node == &renamed ? newName : named->name);
  for (const auto* at = node.parent; at != nullptr; at = at->parent) {
    if (cursiv::ast_cast<Catalogue>(at) || cursiv::ast_cast<Assembly>(at)
        || cursiv::ast_cast<Configuration>(at) || cursiv::ast_cast<LinkBase>(at)
        || cursiv::ast_cast<Schedule>(at))
      break;
    if (const auto* parent = cursiv::ast_cast<NamedElement>(at)) {
      name = std::string(at == &renamed ? newName : parent->name) + "." + name;
    }
  }
  return name;
}

// Whether `node` is `ancestor` or lies under it.
bool below(const cursiv::AstNode& node, const cursiv::AstNode& ancestor) {
  for (const auto* at = &node; at != nullptr; at = at->parent)
    if (at == &ancestor) return true;
  return false;
}

// What the document spells between two offsets, without the whitespace and
// the comments the grammar hides.
std::string spelled(const cursiv::Document& document, cursiv::TextOffset begin,
                    cursiv::TextOffset end) {
  const auto& cst = document.parseResult.cst();
  std::string text;
  for (auto rank = cst.leaves_before(begin); rank < cst.leaf_count(); ++rank) {
    const auto leaf = cst.leaf(rank);
    if (leaf.begin >= end) break;
    if (leaf.begin >= begin && !leaf.hidden) text += cst.text(leaf.id);
  }
  return text;
}

}  // namespace

std::optional<::lsp::Range> XsmpRenameProvider::prepareRename(
    const cursiv::Document& document, ::lsp::Position position,
    const cursiv::CancellationToken& token) const {
  if (isReadOnlyDocumentUri(document.uri)) return std::nullopt;
  const auto found = core().references->findDeclaration(
      document, document.textDocument().offsetAt(cursiv::from_lsp(position)),
      token);
  // An unresolved reference is not renamed, unlike in the default provider:
  // unchecked deployment paths would become rename targets.
  if (!found) return std::nullopt;
  for (const auto* target : found.targets)
    if (const auto* home = target->getDocument();
        home && isReadOnlyDocumentUri(home->uri))
      return std::nullopt;
  return cursiv::to_lsp(
      document.textDocument().rangeAt(found.span.begin, found.span.end));
}

std::optional<::lsp::WorkspaceEdit> XsmpRenameProvider::rename(
    const cursiv::Document& document, ::lsp::Position position,
    std::string_view newName, const cursiv::CancellationToken& token) const {
  if (!prepareRename(document, position, token)) return std::nullopt;
  auto edit =
      cursiv::DefaultRenameProvider::rename(document, position, newName, token);
  if (!edit) return edit;

  const auto declaration = core().references->findDeclaration(
      document, document.textDocument().offsetAt(cursiv::from_lsp(position)),
      token);
  if (!declaration || declaration.targets.empty()) return edit;

  // The default renamed the name itself. What is left are the references that
  // spell a qualified name, of the target or of anything below it.
  if (!edit->changes) edit->changes.emplace();
  for (const auto* target : declaration.targets) {
    const auto* targetName = cursiv::ast_cast<NamedElement>(target);
    if (!targetName) continue;
    const auto* home = target->getDocument();
    if (!home) continue;
    for (const auto* node : home->parseResult.nodes()) {
      cursiv::throw_if_cancelled(token);
      if (!below(*node, *target)) continue;
      const auto* named = cursiv::ast_cast<NamedElement>(node);
      if (!named) continue;
      const auto qualified = qualifiedAfterRename(*node, *target, newName);
      const auto original =
          qualifiedAfterRename(*node, *target, targetName->name);
      for (const auto& reference :
           core().references->findReferences(*node, {}, token)) {
        const auto source =
            core().shared().documents->getDocument(reference.sourceDocumentId);
        if (!source) continue;
        const auto& text = source->textDocument();
        if (reference.sourceEnd > text.getText().size()) continue;
        // Match the same qualified suffix the author used. References in a
        // nested namespace may spell b.Type rather than its full a.b.Type.
        const auto normalized =
            spelled(*source, reference.sourceBegin, reference.sourceEnd);
        auto oldName = original, replacement = qualified;
        while (oldName != replacement && oldName != normalized) {
          const auto oldDot = oldName.find('.');
          const auto newDot = replacement.find('.');
          if (oldDot == std::string::npos || newDot == std::string::npos) break;
          oldName.erase(0, oldDot + 1);
          replacement.erase(0, newDot + 1);
        }
        if (oldName == replacement || oldName != normalized) continue;

        ::lsp::TextEdit rewrite;
        rewrite.range = cursiv::to_lsp(
            text.rangeAt(reference.sourceBegin, reference.sourceEnd));
        rewrite.newText = std::move(replacement);
        auto& changes =
            (*edit->changes)[::lsp::DocumentUri::parse(source->uri)];
        // It takes the place of the default's edit at the same start.
        const auto existing =
            std::ranges::find_if(changes, [&](const auto& other) {
              return other.range.start.line == rewrite.range.start.line
                     && other.range.start.character
                            == rewrite.range.start.character;
            });
        if (existing == changes.end())
          changes.push_back(std::move(rewrite));
        else
          *existing = std::move(rewrite);
      }
    }
  }
  return edit;
}

}  // namespace xsmp
