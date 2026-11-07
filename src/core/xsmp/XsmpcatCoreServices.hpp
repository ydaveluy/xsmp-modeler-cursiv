#pragma once

#include <cursiv/services/CoreServices.hpp>
#include <cursiv/services/CoreServicesFor.hpp>
#include <memory>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/references/QualifiedName.hpp>
#include <xsmp/references/XsmpTypeProvider.hpp>
#include <xsmp/validation/XsmpcatValidator.hpp>

namespace xsmp {

/// Core-only xsmpcat language services (used by the CLI and headless
/// scenarios), built from the shared xsmp core type. The LSP container extends
/// it with `cursiv::LspServices`.
struct XsmpcatCoreServices : cursiv::CoreServicesFor<XsmpSharedCoreServices> {
  std::unique_ptr<const XsmpTypeProvider> typeProvider;
  std::unique_ptr<XsmpcatValidator> validator;

  using CoreServicesFor::CoreServicesFor;
};

}  // namespace xsmp
