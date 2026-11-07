#pragma once

#include <cursiv/services/LspServicesFor.hpp>
#include <xsmp/SharedLspServices.hpp>
#include <xsmp/XsmpsedCoreServices.hpp>

namespace xsmp {

struct XsmpsedServices final
    : cursiv::LspServicesFor<XsmpsedCoreServices, XsmpSharedServices> {
  using LspServicesFor::LspServicesFor;
};

}  // namespace xsmp
