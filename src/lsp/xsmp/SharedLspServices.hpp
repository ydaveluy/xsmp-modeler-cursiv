#pragma once

#include <cursiv/services/SharedLspServices.hpp>
#include <xsmp/SharedCoreServices.hpp>

namespace xsmp {

struct XsmpSharedServices final : XsmpSharedCoreServices,
                                  cursiv::SharedLspServices {};

}  // namespace xsmp
