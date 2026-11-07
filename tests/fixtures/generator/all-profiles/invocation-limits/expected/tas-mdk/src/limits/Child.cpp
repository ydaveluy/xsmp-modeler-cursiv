// -----------------------------------------------------------------------------
// File Name    : Child.cpp
// -----------------------------------------------------------------------------
/// @file limits/Child.cpp

#include <limits/Child.h>

namespace limits
{
Child::Child(
        ::Smp::String8 name,
        ::Smp::String8 description,
        ::Smp::IObject* parent,
        ::Smp::Publication::ITypeRegistry* type_registry)
        : ChildGen::ChildGen(name, description, parent, type_registry) {
    // Publish_Callback = std::bind(&Child::_publishHook, this, std::placeholders::_1);

    // Configure_Callback = std::bind(&Child::_configureHook, this, std::placeholders::_1, std::placeholders::_2);

    // Connect_Callback = std::bind(&Child::_connectHook, this, std::placeholders::_1);

    // Disconnect_Callback = std::bind(&Child::_disconnectHook, this);
}

::Smp::Int32 Child::normal(::Smp::Float64 value) {
    // TODO
    return 0;
}

::Smp::Int32 Child::fresh() {
    // TODO
    return 0;
}
} // namespace limits
