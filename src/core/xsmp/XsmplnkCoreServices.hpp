#pragma once

#include <cursiv/services/CoreServicesFor.hpp>
#include <memory>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/validation/XsmplnkValidator.hpp>

namespace xsmp {

// Core-only xsmplnk language services. The LSP container extends it.
struct XsmplnkCoreServices : cursiv::CoreServicesFor<XsmpSharedCoreServices> {
  std::unique_ptr<XsmplnkValidator> validator;

  using CoreServicesFor::CoreServicesFor;
};

}  // namespace xsmp
