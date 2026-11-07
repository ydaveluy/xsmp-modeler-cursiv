// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Serv.cpp
// -----------------------------------------------------------------------------
/// @file m/Serv.cpp

#include <m/Serv.h>

namespace m
{
Serv::Serv(
        ::Smp::String8 name,
        ::Smp::String8 description,
        ::Smp::IObject* parent,
        ::Smp::Publication::ITypeRegistry* type_registry)
        : ServGen::ServGen(name, description, parent, type_registry) {
    // Publish_Callback = std::bind(&Serv::_publishHook, this, std::placeholders::_1);

    // Configure_Callback = std::bind(&Serv::_configureHook, this, std::placeholders::_1, std::placeholders::_2);

    // Connect_Callback = std::bind(&Serv::_connectHook, this, std::placeholders::_1);

    // Disconnect_Callback = std::bind(&Serv::_disconnectHook, this);
}

::Smp::Bool Serv::ping(::Smp::Int32 value) {
    // TODO
    return false;
}

::Smp::Int32 Serv::get_ro(){
    ::Smp::Int32 ret {};
    return ret;
}

void Serv::set_wo(::Smp::Int32 value){
    // TODO
}

void Serv::_go() {
    // TODO implement EntryPoint go
}

void Serv::_bang(::Smp::IObject* sender) {
    // TODO implement EventSink bang
}
} // namespace m
