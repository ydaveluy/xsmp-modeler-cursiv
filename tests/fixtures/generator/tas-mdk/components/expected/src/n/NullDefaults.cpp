// -----------------------------------------------------------------------------
// File Name    : NullDefaults.cpp
// -----------------------------------------------------------------------------
/// @file n/NullDefaults.cpp

#include <n/NullDefaults.h>

namespace n
{
NullDefaults::NullDefaults(
        ::Smp::String8 name,
        ::Smp::String8 description,
        ::Smp::IObject* parent,
        ::Smp::Publication::ITypeRegistry* type_registry)
        : NullDefaultsGen::NullDefaultsGen(name, description, parent, type_registry) {
    // Publish_Callback = std::bind(&NullDefaults::_publishHook, this, std::placeholders::_1);

    // Configure_Callback = std::bind(&NullDefaults::_configureHook, this, std::placeholders::_1, std::placeholders::_2);

    // Connect_Callback = std::bind(&NullDefaults::_connectHook, this, std::placeholders::_1);

    // Disconnect_Callback = std::bind(&NullDefaults::_disconnectHook, this);
}

void NullDefaults::ping(::Smp::Int32* a, ::Smp::Int32 b, ::Smp::Int32* c, ::Smp::Char8 d) {
    // TODO
}
} // namespace n
