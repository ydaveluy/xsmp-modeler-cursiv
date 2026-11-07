#include <cursiv/text/LspPosition.hpp>
#include <cursiv/validation/DocumentValidator.hpp>
#include <limits>
#include <unordered_map>
#include <variant>
#include <xsmp/XsmpCodeActionProvider.hpp>
#include <xsmp/XsmpUuid.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/validation/IssueCodes.hpp>
#include <xsmp/workspace/SourceUri.hpp>

namespace xsmp {

namespace {

// The issue code of a diagnostic, from its `code` or from the `code` its `data`
// carries. Empty where it has neither.
std::string diagnosticCode(const ::lsp::Diagnostic& diagnostic) {
  if (diagnostic.code)
    if (const auto* code = std::get_if<std::string>(&*diagnostic.code))
      return *code;
  if (diagnostic.data && diagnostic.data->isObject())
    if (const auto* code = diagnostic.data->object().find("code");
        code && code->isString())
      return code->string();
  return {};
}

}  // namespace

void XsmpCodeActionProvider::appendCodeActions(
    const cursiv::Document& document, const ::lsp::CodeActionParams& params,
    std::vector<::lsp::CodeAction>& actions,
    const cursiv::CancellationToken& token) const {
  if (isReadOnlyDocumentUri(document.uri)) return;

  const auto text = document.textDocument().getText();
  const auto& cst = document.parseResult.cst();
  const auto add = [&](std::string title, const ::lsp::Diagnostic& diagnostic,
                       ::lsp::Range range, std::string replacement) {
    ::lsp::CodeAction action;
    action.title = std::move(title);
    action.kind = ::lsp::CodeActionKind::QuickFix;
    action.isPreferred = true;
    action.diagnostics = std::vector<::lsp::Diagnostic>{diagnostic};
    ::lsp::TextEdit edit;
    edit.range = range;
    edit.newText = std::move(replacement);
    ::lsp::WorkspaceEdit workspace;
    workspace.changes.emplace();
    (*workspace.changes)[::lsp::DocumentUri::parse(document.uri)].push_back(
        std::move(edit));
    action.edit = std::move(workspace);
    actions.push_back(std::move(action));
  };

  // The `unsafe` action of each node: diagnostics on one path share it.
  std::unordered_map<cursiv::NodeId, std::size_t> unsafeNodes;
  for (const auto& diagnostic : params.context.diagnostics) {
    cursiv::throw_if_cancelled(token);
    const auto code = diagnosticCode(diagnostic);
    const auto begin = document.textDocument().offsetAt(
        cursiv::from_lsp(diagnostic.range.start));
    const auto end = document.textDocument().offsetAt(
        cursiv::from_lsp(diagnostic.range.end));

    if (code == InvalidUuid || code == DuplicatedUuid) {
      add("Generate new UUID.", diagnostic, diagnostic.range, newUuid());
    } else if (code == IllegalModifier || code == InvalidModifier
               || code == InvalidAttribute || code == InvalidUsage
               || code == DuplicatedUsage) {
      add("Remove " + std::string(text.substr(begin, end - begin)), diagnostic,
          diagnostic.range, "");
    } else if (code == MissingAbstract) {
      add("Add `abstract` keyword.", diagnostic,
          {.start = diagnostic.range.start, .end = diagnostic.range.start},
          "abstract ");
    } else if (code == EnumLiteralExpected && diagnostic.relatedInformation
               && !diagnostic.relatedInformation->empty()) {
      add("Replace by Enumeration literal.", diagnostic, diagnostic.range,
          diagnostic.relatedInformation->front().message);
    } else if (code == MissingUuid) {
      // The type the diagnostic lies in.
      const Type* type = nullptr;
      for (const auto* node : document.parseResult.nodes())
        if (const auto* candidate = cursiv::ast_cast<Type>(node);
            candidate && candidate->cstNodeId != cursiv::kNoNode
            && cst[candidate->cstNodeId].begin <= begin
            && cst[candidate->cstNodeId].end >= end) {
          type = candidate;
          break;
        }
      if (!type) continue;

      const auto edit = uuidEdit(document, cst[type->cstNodeId].begin);
      add("Generate missing UUID.", diagnostic, edit.range, edit.newText);
    } else if (code != cursiv::kParseRepaired && code != cursiv::kSyntaxError
               && code != cursiv::kConversionError) {
      // Any other issue in a checked path, but for the syntax around it: offer
      // to mark the innermost such path as `unsafe`.
      const cursiv::AstNode* selected = nullptr;
      auto width = std::numeric_limits<cursiv::TextOffset>::max();
      for (const auto* node : document.parseResult.nodes()) {
        if (node->cstNodeId == cursiv::kNoNode) continue;
        const auto* path = cursiv::ast_cast<Path>(node);
        const auto* local = cursiv::ast_cast<LocalNamedReference>(node);
        const auto* field = cursiv::ast_cast<ConcretePathNamedSegment>(node);
        if (!(path && !path->unsafe) && !(local && !local->unsafe)
            && !(field && !field->unsafe
                 && cursiv::ast_cast<CfgStructureFieldValue>(node->parent)))
          continue;
        const auto& record = cst[node->cstNodeId];
        if (record.begin <= begin && record.end >= end
            && record.end - record.begin < width) {
          selected = node;
          width = record.end - record.begin;
        }
      }

      if (selected) {
        if (const auto previous = unsafeNodes.find(selected->cstNodeId);
            previous != unsafeNodes.end()) {
          actions[previous->second].diagnostics->push_back(diagnostic);
          continue;
        }
        unsafeNodes.emplace(selected->cstNodeId, actions.size());
        const auto at = cursiv::to_lsp(
            document.textDocument().positionAt(cst[selected->cstNodeId].begin));
        add("Declare as `unsafe`.", diagnostic, {.start = at, .end = at},
            "unsafe ");
      }
    }
  }
}

}  // namespace xsmp
