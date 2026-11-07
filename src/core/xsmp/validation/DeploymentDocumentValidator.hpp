#pragma once

#include <xsmp/validation/XsmpDocumentValidator.hpp>

namespace xsmp {

// The linking diagnostics of the deployment languages, without those of path
// segments and local references: `checkPath` and the checks of each language
// report these in their own terms, and an `unsafe` one need not resolve.
class DeploymentDocumentValidator final : public XsmpDocumentValidator {
 public:
  using XsmpDocumentValidator::XsmpDocumentValidator;

 protected:
  void processLinkingErrors(
      const cursiv::Document& document, std::vector<cursiv::Diagnostic>& out,
      const cursiv::CancellationToken& token) const override;
};

}  // namespace xsmp
