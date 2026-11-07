// -----------------------------------------------------------------------------
// File Name    : Device.cpp
// -----------------------------------------------------------------------------
/// @file n/Device.cpp

#include <n/Device.h>
// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/I.h>

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

::n::I* Device::target() {
    // TODO
    return nullptr;
}

::Smp::Bool Device::get_ready() const{
    ::Smp::Bool ret {};
    return ret;
}
} // namespace n
