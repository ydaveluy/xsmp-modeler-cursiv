#pragma once

#include <cursiv/services/CoreServices.hpp>
#include <cursiv/services/CoreServicesFor.hpp>
#include <memory>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/validation/XsmpprojectValidator.hpp>

namespace xsmp {

// Core-only `xsmp.project` language services. The LSP container extends it.
struct XsmpprojectCoreServices
    : cursiv::CoreServicesFor<XsmpSharedCoreServices> {
  std::unique_ptr<XsmpprojectValidator> validator;

  using CoreServicesFor::CoreServicesFor;
};

}  // namespace xsmp
