// -----------------------------------------------------------------------------
// File Name    : M.cpp
// -----------------------------------------------------------------------------
/// @file n/M.cpp

#include <n/M.h>

namespace n
{
M::M(
        ::Smp::String8 name,
        ::Smp::String8 description,
        ::Smp::IObject* parent,
        ::Smp::Publication::ITypeRegistry* type_registry)
        : MGen::MGen(name, description, parent, type_registry) {
    // Publish_Callback = std::bind(&M::_publishHook, this, std::placeholders::_1);

    // Configure_Callback = std::bind(&M::_configureHook, this, std::placeholders::_1, std::placeholders::_2);

    // Connect_Callback = std::bind(&M::_connectHook, this, std::placeholders::_1);

    // Disconnect_Callback = std::bind(&M::_disconnectHook, this);
}

void M::ope_reset() {
    // TODO
}

void M::_ept_step() {
    // TODO implement EntryPoint ept_step
}
} // namespace n
