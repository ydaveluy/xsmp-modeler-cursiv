// -----------------------------------------------------------------------------
// File Name    : Probe.cpp
// -----------------------------------------------------------------------------
/// @file defaults/Probe.cpp

#include <defaults/Probe.h>

namespace defaults
{
Probe::Probe(
        ::Smp::String8 name,
        ::Smp::String8 description,
        ::Smp::IObject* parent,
        ::Smp::Publication::ITypeRegistry* type_registry)
        : ProbeGen::ProbeGen(name, description, parent, type_registry) {
    // Publish_Callback = std::bind(&Probe::_publishHook, this, std::placeholders::_1);

    // Configure_Callback = std::bind(&Probe::_configureHook, this, std::placeholders::_1, std::placeholders::_2);

    // Connect_Callback = std::bind(&Probe::_connectHook, this, std::placeholders::_1);

    // Disconnect_Callback = std::bind(&Probe::_disconnectHook, this);
}
} // namespace defaults
