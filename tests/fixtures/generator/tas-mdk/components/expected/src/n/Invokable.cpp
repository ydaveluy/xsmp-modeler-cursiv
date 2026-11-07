// -----------------------------------------------------------------------------
// File Name    : Invokable.cpp
// -----------------------------------------------------------------------------
/// @file n/Invokable.cpp

#include <n/Invokable.h>

namespace n
{
Invokable::Invokable(
        ::Smp::String8 name,
        ::Smp::String8 description,
        ::Smp::IObject* parent,
        ::Smp::Publication::ITypeRegistry* type_registry)
        : InvokableGen::InvokableGen(name, description, parent, type_registry) {
    // Publish_Callback = std::bind(&Invokable::_publishHook, this, std::placeholders::_1);

    // Configure_Callback = std::bind(&Invokable::_configureHook, this, std::placeholders::_1, std::placeholders::_2);

    // Connect_Callback = std::bind(&Invokable::_connectHook, this, std::placeholders::_1);

    // Disconnect_Callback = std::bind(&Invokable::_disconnectHook, this);
}

void Invokable::ping(::Smp::Int32 count) {
    // TODO
}

::Smp::Int32* Invokable::get_pointer(){
    ::Smp::Int32* ret {};
    return ret;
}
void Invokable::set_pointer(::Smp::Int32* value){
    // TODO
}

::Smp::Int32 Invokable::get_plain(){
    ::Smp::Int32 ret {};
    return ret;
}
void Invokable::set_plain(::Smp::Int32 value){
    // TODO
}
} // namespace n
