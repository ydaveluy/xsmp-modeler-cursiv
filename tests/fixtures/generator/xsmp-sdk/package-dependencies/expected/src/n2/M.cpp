// A field of `other` and an operation of `types` chain their packages. An
// interface has no implementation, so that `interfaces` is not chained.
// -----------------------------------------------------------------------------
// File Name    : M.cpp
// -----------------------------------------------------------------------------
/// @file n2/M.cpp

#include <n2/M.h>

namespace n2
{
void M::DoPublish(::Smp::IPublication* receiver) {
}

void M::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void M::DoConnect(::Smp::ISimulator* simulator) {
}

void M::DoDisconnect() {
}

::t::T M::f(::t::T x) {
    // TODO
    return 0;
}
} // namespace n2
