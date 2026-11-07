// -----------------------------------------------------------------------------
// File Name    : Probe.cpp
// -----------------------------------------------------------------------------
/// @file optional/Probe.cpp

#include <optional/Probe.h>

namespace optional
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
