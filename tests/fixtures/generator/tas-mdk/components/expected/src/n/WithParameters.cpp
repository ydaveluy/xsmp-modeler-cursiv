// -----------------------------------------------------------------------------
// File Name    : WithParameters.cpp
// -----------------------------------------------------------------------------
/// @file n/WithParameters.cpp

#include <n/WithParameters.h>

namespace n
{
WithParameters::WithParameters(
        ::Smp::String8 name,
        ::Smp::String8 description,
        ::Smp::IObject* parent,
        ::Smp::Publication::ITypeRegistry* type_registry)
        : WithParametersGen::WithParametersGen(name, description, parent, type_registry) {
    // Publish_Callback = std::bind(&WithParameters::_publishHook, this, std::placeholders::_1);

    // Configure_Callback = std::bind(&WithParameters::_configureHook, this, std::placeholders::_1, std::placeholders::_2);

    // Connect_Callback = std::bind(&WithParameters::_connectHook, this, std::placeholders::_1);

    // Disconnect_Callback = std::bind(&WithParameters::_disconnectHook, this);
}

void WithParameters::ping(::Smp::Int32 count) {
    // TODO
}
} // namespace n
