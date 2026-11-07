// -----------------------------------------------------------------------------
// File Name    : Device.cpp
// -----------------------------------------------------------------------------
/// @file n/Device.cpp

#include <n/Device.h>

namespace n
{
void Device::DoPublish(::Smp::IPublication* receiver) {
}

void Device::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void Device::DoConnect(::Smp::ISimulator* simulator) {
}

void Device::DoDisconnect() {
}

void Device::invisible() {
    // TODO
}

void Device::measure(::Smp::Float64* value) {
    // TODO
}
} // namespace n
