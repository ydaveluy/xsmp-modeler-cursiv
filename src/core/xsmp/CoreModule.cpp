#include <xsmp/CoreModule.hpp>

namespace xsmp {

bool registerXsmpServices(cursiv::SharedCoreServices& shared) {
  return registerXsmpprojectServices(shared) && registerXsmpcatServices(shared)
         && registerXsmpasbServices(shared) && registerXsmpcfgServices(shared)
         && registerXsmplnkServices(shared) && registerXsmpsedServices(shared);
}

}  // namespace xsmp
