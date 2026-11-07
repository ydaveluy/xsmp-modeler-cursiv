// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Empty.cpp
// -----------------------------------------------------------------------------
/// @file m/Empty.cpp

#include <m/Empty.h>

namespace m
{
Empty::Empty(
        ::Smp::String8 name,
        ::Smp::String8 description,
        ::Smp::IObject* parent,
        ::Smp::Publication::ITypeRegistry* type_registry)
        : EmptyGen::EmptyGen(name, description, parent, type_registry) {
    // Publish_Callback = std::bind(&Empty::_publishHook, this, std::placeholders::_1);

    // Configure_Callback = std::bind(&Empty::_configureHook, this, std::placeholders::_1, std::placeholders::_2);

    // Connect_Callback = std::bind(&Empty::_connectHook, this, std::placeholders::_1);

    // Disconnect_Callback = std::bind(&Empty::_disconnectHook, this);
}
} // namespace m
