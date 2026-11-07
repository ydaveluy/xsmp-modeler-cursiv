// -----------------------------------------------------------------------------
// File Name    : M.cpp
// -----------------------------------------------------------------------------
/// @file n/M.cpp

#include <n/M.h>

#include <stdexcept>

namespace n
{
void M::DoPublish(::Smp::IPublication* receiver) {
}

void M::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void M::DoConnect(::Smp::ISimulator* simulator) {
}

void M::DoDisconnect() {
}

::n::Huge M::huge() {
    // TODO
    return {};
}

::n::S M::structure() {
    // TODO
    return {/*.items = */{}};
}

::n::E M::literal() {
    // TODO
    return ::n::E::A;
}

::Smp::Int32 M::number() {
    // TODO
    return 0;
}

::Smp::Int32& M::reference() {
    // TODO
    throw std::logic_error("Operation is not implemented.");
}

::Smp::Int32& M::get_value(){
    throw std::logic_error("Property getter is not implemented.");
}
void M::set_value(::Smp::Int32& value){
    // TODO
}
} // namespace n
