#pragma once

#include <cursiv/services/LspServicesFor.hpp>
#include <xsmp/SharedLspServices.hpp>
#include <xsmp/XsmpcfgCoreServices.hpp>

namespace xsmp {

struct XsmpcfgServices final
    : cursiv::LspServicesFor<XsmpcfgCoreServices, XsmpSharedServices> {
  using LspServicesFor::LspServicesFor;
};

}  // namespace xsmp
