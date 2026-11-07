#include <algorithm>
#include <cursiv/syntax-tree/DocumentReferences.hpp>
#include <cursiv/workspace/Document.hpp>
#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/validation/DeploymentDocumentValidator.hpp>

namespace xsmp {

namespace {

bool isOptionalPathReference(const ReferenceInfo& reference) {
  return cursiv::ast_cast<ConcretePathNamedSegment>(reference.container)
         || cursiv::ast_cast<LocalNamedReference>(reference.container);
}

}  // namespace

void DeploymentDocumentValidator::processLinkingErrors(
    const cursiv::Document& document, std::vector<cursiv::Diagnostic>& out,
    const cursiv::CancellationToken& token) const {
  std::vector<cursiv::Diagnostic> found;
  XsmpDocumentValidator::processLinkingErrors(document, found, token);

  std::vector<cursiv::TextRange> optional;
  if (document.parseResult.references)
    document.parseResult.references->visitUnresolved(
        document.parseResult.cst(),
        [&](cursiv::RefState, cursiv::TextOffset begin, cursiv::TextOffset end,
            const ReferenceInfo& reference) {
          if (isOptionalPathReference(reference))
            optional.push_back({.begin = begin, .end = end});
        });

  // A diagnostic does not say which reference it came from: match by range.
  for (auto& d : found) {
    const auto path = std::ranges::any_of(optional, [&](auto range) {
      return range.begin == d.range.begin && range.end == d.range.end;
    });
    if (!path) out.push_back(std::move(d));
  }
}

}  // namespace xsmp
