// -----------------------------------------------------------------------------
// File Name    : Probe.cpp
// -----------------------------------------------------------------------------
/// @file limits/Probe.cpp

#include <limits/Probe.h>

namespace limits
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
