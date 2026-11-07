#pragma once

#include <array>
#include <cursiv/content/TextDocumentContentProvider.hpp>
#include <cursiv/services/SharedCoreServices.hpp>
#include <optional>
#include <string>
#include <string_view>

namespace xsmp {

// Gives the editor the text of documents no file holds: the libraries (`xsmp:`)
// and the read-only XSMP form of SMP XML files (`xsmp-smp:`).
class XsmpTextDocumentContentProvider final
    : public cursiv::TextDocumentContentProvider {
 public:
  explicit XsmpTextDocumentContentProvider(
      const cursiv::SharedCoreServices& shared)
      : shared(shared) {}

  std::span<const std::string> schemes() const noexcept override {
    return supported;
  }

  std::optional<::lsp::TextDocumentContentResult> provideTextDocumentContent(
      const ::lsp::TextDocumentContentParams&) const override;

 private:
  const cursiv::SharedCoreServices& shared;
  const std::array<std::string, 2> supported{"xsmp", "xsmp-smp"};
};

}  // namespace xsmp
