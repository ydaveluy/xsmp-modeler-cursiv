// -----------------------------------------------------------------------------
// File Name    : Probe.cpp
// -----------------------------------------------------------------------------
/// @file defaults/Probe.cpp

#include <defaults/Probe.h>

namespace defaults
{
void Probe::DoPublish(::Smp::IPublication* receiver) {
}

void Probe::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void Probe::DoConnect(::Smp::ISimulator* simulator) {
}

void Probe::DoDisconnect() {
}
} // namespace defaults
