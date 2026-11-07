// -----------------------------------------------------------------------------
// File Name    : Bare.cpp
// -----------------------------------------------------------------------------
/// @file n/Bare.cpp

#include <n/Bare.h>

namespace n
{
Bare::Bare(
        ::Smp::String8 name,
        ::Smp::String8 description,
        ::Smp::IObject* parent,
        ::Smp::Publication::ITypeRegistry* type_registry)
        : BareGen::BareGen(name, description, parent, type_registry) {
    // Publish_Callback = std::bind(&Bare::_publishHook, this, std::placeholders::_1);

    // Configure_Callback = std::bind(&Bare::_configureHook, this, std::placeholders::_1, std::placeholders::_2);

    // Connect_Callback = std::bind(&Bare::_connectHook, this, std::placeholders::_1);

    // Disconnect_Callback = std::bind(&Bare::_disconnectHook, this);
}

void Bare::reset() {
    // TODO
}
} // namespace n
