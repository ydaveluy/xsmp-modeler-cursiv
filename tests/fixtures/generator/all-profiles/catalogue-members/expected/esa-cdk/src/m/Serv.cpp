// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Serv.cpp
// -----------------------------------------------------------------------------
/// @file m/Serv.cpp

#include <m/Serv.h>

namespace m
{
void Serv::DoPublish(::Smp::IPublication* receiver) {
}

void Serv::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void Serv::DoConnect(::Smp::ISimulator* simulator) {
}

void Serv::DoDisconnect() {
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
