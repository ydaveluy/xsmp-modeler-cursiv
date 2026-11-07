#pragma once

#include <cursiv/services/LspServicesFor.hpp>
#include <xsmp/SharedLspServices.hpp>
#include <xsmp/XsmpcatCoreServices.hpp>

namespace xsmp {

struct XsmpcatServices final
    : cursiv::LspServicesFor<XsmpcatCoreServices, XsmpSharedServices> {
  using LspServicesFor::LspServicesFor;
};

}  // namespace xsmp
