#pragma once

#include <cursiv/services/CoreServicesFor.hpp>
#include <memory>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/validation/XsmpsedValidator.hpp>

namespace xsmp {

// Core-only xsmpsed language services. The LSP container extends it.
struct XsmpsedCoreServices : cursiv::CoreServicesFor<XsmpSharedCoreServices> {
  std::unique_ptr<XsmpsedValidator> validator;

  using CoreServicesFor::CoreServicesFor;
};

}  // namespace xsmp
