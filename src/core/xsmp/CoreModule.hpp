#pragma once

#include <cursiv/services/SharedCoreServices.hpp>
#include <xsmp/XsmpasbCoreModule.hpp>
#include <xsmp/XsmpcatCoreModule.hpp>
#include <xsmp/XsmpcfgCoreModule.hpp>
#include <xsmp/XsmplnkCoreModule.hpp>
#include <xsmp/XsmpprojectCoreModule.hpp>
#include <xsmp/XsmpsedCoreModule.hpp>

namespace xsmp {

// Register the complete language family, in workspace dependency order.
bool registerXsmpServices(cursiv::SharedCoreServices& shared);

}  // namespace xsmp
