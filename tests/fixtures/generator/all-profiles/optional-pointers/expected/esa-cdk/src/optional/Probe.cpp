// -----------------------------------------------------------------------------
// File Name    : Probe.cpp
// -----------------------------------------------------------------------------
/// @file optional/Probe.cpp

#include <optional/Probe.h>

namespace optional
{
void Probe::DoPublish(::Smp::IPublication* receiver) {
}

void Probe::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void Probe::DoConnect(::Smp::ISimulator* simulator) {
}

void Probe::DoDisconnect() {
}

void Probe::ope_input(::Smp::Int32* value) {
    // TODO
}

void Probe::ope_string(::Smp::String8* value) {
    // TODO
}

void Probe::ope_output(::Smp::Int32* value) {
    // TODO
}

void Probe::ope_inout(::Smp::Int32* value) {
    // TODO
}

void Probe::ope_names(::Smp::Int32* value, ::Smp::Int32* value_present) {
    // TODO
}
} // namespace optional
