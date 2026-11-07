// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : OnlyOutput.cpp
// -----------------------------------------------------------------------------
/// @file m/OnlyOutput.cpp

#include <m/OnlyOutput.h>

namespace m
{
OnlyOutput::OnlyOutput(
        ::Smp::String8 name,
        ::Smp::String8 description,
        ::Smp::IObject* parent,
        ::Smp::Publication::ITypeRegistry* type_registry)
        : OnlyOutputGen::OnlyOutputGen(name, description, parent, type_registry) {
    // Publish_Callback = std::bind(&OnlyOutput::_publishHook, this, std::placeholders::_1);

    // Configure_Callback = std::bind(&OnlyOutput::_configureHook, this, std::placeholders::_1, std::placeholders::_2);

    // Connect_Callback = std::bind(&OnlyOutput::_connectHook, this, std::placeholders::_1);

    // Disconnect_Callback = std::bind(&OnlyOutput::_disconnectHook, this);
}
} // namespace m
