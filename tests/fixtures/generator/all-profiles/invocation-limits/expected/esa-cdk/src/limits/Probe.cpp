// -----------------------------------------------------------------------------
// File Name    : Probe.cpp
// -----------------------------------------------------------------------------
/// @file limits/Probe.cpp

#include <limits/Probe.h>

namespace limits
{
void Probe::DoPublish(::Smp::IPublication* receiver) {
}

void Probe::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void Probe::DoConnect(::Smp::ISimulator* simulator) {
}

void Probe::DoDisconnect() {
}

::Smp::Int32* Probe::pointer() {
    // TODO
    return nullptr;
}

::Smp::Int32 Probe::calculate(::Smp::Int32 value) {
    // TODO
    return 0;
}

::Smp::Int32 Probe::calculate(::Smp::Float64 value) {
    // TODO
    return 0;
}

::Smp::Int32 Probe::normal(::Smp::Int32 value) {
    // TODO
    return 0;
}
} // namespace limits
