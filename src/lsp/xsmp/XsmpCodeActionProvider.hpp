#pragma once

#include <cursiv/actions/DefaultCodeActionProvider.hpp>

namespace xsmp {

// Quick fixes for XSMP diagnostics: a new or missing `@uuid`, removing an
// illegal modifier, attribute or usage, adding `abstract`, the right
// enumeration literal, and `unsafe` on a path that does not check out.
class XsmpCodeActionProvider final : public cursiv::DefaultCodeActionProvider {
 protected:
  void appendCodeActions(const cursiv::Document&,
                         const ::lsp::CodeActionParams&,
                         std::vector<::lsp::CodeAction>&,
                         const cursiv::CancellationToken&) const override;
};

}  // namespace xsmp
