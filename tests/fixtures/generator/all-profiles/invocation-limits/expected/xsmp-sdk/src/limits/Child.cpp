// -----------------------------------------------------------------------------
// File Name    : Child.cpp
// -----------------------------------------------------------------------------
/// @file limits/Child.cpp

#include <limits/Child.h>

namespace limits
{
void Child::DoPublish(::Smp::IPublication* receiver) {
}

void Child::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void Child::DoConnect(::Smp::ISimulator* simulator) {
}

void Child::DoDisconnect() {
}

::Smp::Int32 Child::normal(::Smp::Float64 value) {
    // TODO
    return 0;
}

::Smp::Int32 Child::fresh() {
    // TODO
    return 0;
}
} // namespace limits
