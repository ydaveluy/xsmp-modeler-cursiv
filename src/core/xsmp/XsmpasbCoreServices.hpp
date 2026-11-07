#pragma once

#include <cursiv/services/CoreServicesFor.hpp>
#include <memory>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/validation/XsmpasbValidator.hpp>

namespace xsmp {

// Core-only xsmpasb language services. The LSP container extends it.
struct XsmpasbCoreServices : cursiv::CoreServicesFor<XsmpSharedCoreServices> {
  std::unique_ptr<XsmpasbValidator> validator;

  using CoreServicesFor::CoreServicesFor;
};

}  // namespace xsmp
