#pragma once

#include <cursiv/services/LspServicesFor.hpp>
#include <xsmp/SharedLspServices.hpp>
#include <xsmp/XsmpprojectCoreServices.hpp>

namespace xsmp {

struct XsmpprojectServices final
    : cursiv::LspServicesFor<XsmpprojectCoreServices, XsmpSharedServices> {
  using LspServicesFor::LspServicesFor;
};

}  // namespace xsmp
