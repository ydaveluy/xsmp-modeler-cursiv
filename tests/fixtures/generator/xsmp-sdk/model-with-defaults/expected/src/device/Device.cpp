// -----------------------------------------------------------------------------
// File Name    : Device.cpp
// -----------------------------------------------------------------------------
/// @file device/Device.cpp

#include <device/Device.h>
// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <device/ITarget.h>

namespace device
{
void Device::DoPublish(::Smp::IPublication* receiver) {
}

void Device::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void Device::DoConnect(::Smp::ISimulator* simulator) {
}

void Device::DoDisconnect() {
}

::Smp::Bool Device::ready() {
    // TODO
    return false;
}

void Device::setLabel(::Smp::String8 value) {
    // TODO
}

::Smp::Bool Device::get_enabled(){
    ::Smp::Bool ret {};
    return ret;
}
void Device::set_enabled(::Smp::Bool value){
    // TODO
}

::Smp::Bool Device::get_immutable() const{
    ::Smp::Bool ret {};
    return ret;
}

::device::ITarget* Device::target() {
    // TODO
    return nullptr;
}
} // namespace device
