#pragma once

#include <cursiv/services/LspServicesFor.hpp>
#include <xsmp/SharedLspServices.hpp>
#include <xsmp/XsmplnkCoreServices.hpp>

namespace xsmp {

struct XsmplnkServices final
    : cursiv::LspServicesFor<XsmplnkCoreServices, XsmpSharedServices> {
  using LspServicesFor::LspServicesFor;
};

}  // namespace xsmp
