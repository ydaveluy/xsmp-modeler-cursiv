// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : ServKid.cpp
// -----------------------------------------------------------------------------
/// @file m/ServKid.cpp

#include <m/ServKid.h>

namespace m
{
ServKid::ServKid(
        ::Smp::String8 name,
        ::Smp::String8 description,
        ::Smp::IObject* parent,
        ::Smp::Publication::ITypeRegistry* type_registry)
        : ServKidGen::ServKidGen(name, description, parent, type_registry) {
    // Publish_Callback = std::bind(&ServKid::_publishHook, this, std::placeholders::_1);

    // Configure_Callback = std::bind(&ServKid::_configureHook, this, std::placeholders::_1, std::placeholders::_2);

    // Connect_Callback = std::bind(&ServKid::_connectHook, this, std::placeholders::_1);

    // Disconnect_Callback = std::bind(&ServKid::_disconnectHook, this);
}
} // namespace m
