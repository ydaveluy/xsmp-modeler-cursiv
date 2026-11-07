#pragma once

#include <cursiv/services/LspServicesFor.hpp>
#include <xsmp/SharedLspServices.hpp>
#include <xsmp/XsmpasbCoreServices.hpp>

namespace xsmp {

struct XsmpasbServices final
    : cursiv::LspServicesFor<XsmpasbCoreServices, XsmpSharedServices> {
  using LspServicesFor::LspServicesFor;
};

}  // namespace xsmp
