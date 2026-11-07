// -----------------------------------------------------------------------------
// File Name    : Pointers.cpp
// -----------------------------------------------------------------------------
/// @file n/Pointers.cpp

#include <n/Pointers.h>

#include <stdexcept>

namespace n
{
void Pointers::DoPublish(::Smp::IPublication* receiver) {
}

void Pointers::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void Pointers::DoConnect(::Smp::ISimulator* simulator) {
}

void Pointers::DoDisconnect() {
}

::Smp::Int32* Pointers::get_pointer(){
    ::Smp::Int32* ret {};
    return ret;
}
void Pointers::set_pointer(::Smp::Int32* value){
    // TODO
}

::Smp::Int32& Pointers::get_reference(){
    throw std::logic_error("Property getter is not implemented.");
}
void Pointers::set_reference(::Smp::Int32& value){
    // TODO
}

::Smp::Int32 Pointers::get_plain(){
    ::Smp::Int32 ret {};
    return ret;
}
void Pointers::set_plain(::Smp::Int32 value){
    // TODO
}

::Smp::Int32* Pointers::address() {
    // TODO
    return nullptr;
}

::Smp::Int32& Pointers::alias() {
    // TODO
    throw std::logic_error("Operation is not implemented.");
}
} // namespace n
