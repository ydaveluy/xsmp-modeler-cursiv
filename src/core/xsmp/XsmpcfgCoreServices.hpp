#pragma once

#include <cursiv/services/CoreServicesFor.hpp>
#include <memory>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/validation/XsmpcfgValidator.hpp>

namespace xsmp {

// Core-only xsmpcfg language services. The LSP container extends it.
struct XsmpcfgCoreServices : cursiv::CoreServicesFor<XsmpSharedCoreServices> {
  std::unique_ptr<XsmpcfgValidator> validator;

  using CoreServicesFor::CoreServicesFor;
};

}  // namespace xsmp
