#pragma once

#include <cursiv/services/SharedLspServices.hpp>
#include <memory>
#include <string>
#include <xsmp/XsmpasbLspServices.hpp>
#include <xsmp/XsmpcfgLspServices.hpp>
#include <xsmp/XsmplnkLspServices.hpp>
#include <xsmp/XsmpsedLspServices.hpp>

namespace xsmp {

std::unique_ptr<XsmpasbServices> createXsmpasbServices(
    XsmpSharedServices& sharedServices, std::string languageId = "xsmpasb");

std::unique_ptr<XsmpcfgServices> createXsmpcfgServices(
    XsmpSharedServices& sharedServices, std::string languageId = "xsmpcfg");

std::unique_ptr<XsmplnkServices> createXsmplnkServices(
    XsmpSharedServices& sharedServices, std::string languageId = "xsmplnk");

std::unique_ptr<XsmpsedServices> createXsmpsedServices(
    XsmpSharedServices& sharedServices, std::string languageId = "xsmpsed");

bool registerXsmpasbServices(cursiv::SharedLspServices& sharedServices);

bool registerXsmpcfgServices(cursiv::SharedLspServices& sharedServices);

bool registerXsmplnkServices(cursiv::SharedLspServices& sharedServices);

bool registerXsmpsedServices(cursiv::SharedLspServices& sharedServices);

}  // namespace xsmp
