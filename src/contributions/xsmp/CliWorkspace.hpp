#pragma once

#include <cursiv/cli.hpp>
#include <xsmp/Builtins.hpp>
#include <xsmp/Contributions.hpp>
#include <xsmp/CoreModule.hpp>
#include <xsmp/XsmpprojectCoreModule.hpp>
#include <xsmp/smp/SmpService.hpp>

namespace xsmp {

// The setup of a process without a language server, such as the command line or
// a worker: the languages, the descriptors of the built-ins, and the SMP
// mirrors.
inline void registerLanguages(XsmpSharedCoreServices& shared) {
  registerXsmpServices(shared);
  loadBuiltinDescriptors(shared);
  installSmpWorkspaceManager(shared);
}

}  // namespace xsmp
