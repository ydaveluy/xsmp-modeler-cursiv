// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : AbstractOne.cpp
// -----------------------------------------------------------------------------
/// @file m/AbstractOne.cpp

#include <m/AbstractOne.h>

namespace m
{
AbstractOne::AbstractOne(
        ::Smp::String8 name,
        ::Smp::String8 description,
        ::Smp::IObject* parent,
        ::Smp::Publication::ITypeRegistry* type_registry)
        : AbstractOneGen::AbstractOneGen(name, description, parent, type_registry) {
    // Publish_Callback = std::bind(&AbstractOne::_publishHook, this, std::placeholders::_1);

    // Configure_Callback = std::bind(&AbstractOne::_configureHook, this, std::placeholders::_1, std::placeholders::_2);

    // Connect_Callback = std::bind(&AbstractOne::_connectHook, this, std::placeholders::_1);

    // Disconnect_Callback = std::bind(&AbstractOne::_disconnectHook, this);
}

::Smp::Bool AbstractOne::ping(::Smp::Int32 value) {
    // TODO
    return false;
}
} // namespace m
