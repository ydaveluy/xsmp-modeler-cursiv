#pragma once

#include <xsmp/Contributions.hpp>

namespace xsmp {

// Describes the TAS-MDK profile. The worker of the extension calls this.
void describeTasMdk(Contribution& contribution, XsmpSharedCoreServices& shared);

// Adds the profile to a workspace that has it built in, as the command line.
void registerTasMdk(XsmpSharedCoreServices& shared);

}  // namespace xsmp
