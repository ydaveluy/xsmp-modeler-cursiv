// -----------------------------------------------------------------------------
// File Name    : Device.cpp
// -----------------------------------------------------------------------------
/// @file n/Device.cpp

#include <n/Device.h>

namespace n
{
Device::Device(
        ::Smp::String8 name,
        ::Smp::String8 description,
        ::Smp::IObject* parent,
        ::Smp::Publication::ITypeRegistry* type_registry)
        : DeviceGen::DeviceGen(name, description, parent, type_registry) {
    // Publish_Callback = std::bind(&Device::_publishHook, this, std::placeholders::_1);

    // Configure_Callback = std::bind(&Device::_configureHook, this, std::placeholders::_1, std::placeholders::_2);

    // Connect_Callback = std::bind(&Device::_connectHook, this, std::placeholders::_1);

    // Disconnect_Callback = std::bind(&Device::_disconnectHook, this);
}

void Device::apply(::Smp::Int32 count) {
    // TODO
}

::Smp::Bool Device::get_ready(){
    ::Smp::Bool ret {};
    return ret;
}
void Device::set_ready(::Smp::Bool value){
    // TODO
}
} // namespace n
