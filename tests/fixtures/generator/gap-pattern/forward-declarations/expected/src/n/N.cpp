// -----------------------------------------------------------------------------
// File Name    : N.cpp
// -----------------------------------------------------------------------------
/// @file n/N.cpp

#include <n/N.h>
// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/I.h>

#include <stdexcept>

namespace n
{
void N::DoPublish(::Smp::IPublication* receiver) {
}

void N::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void N::DoConnect(::Smp::ISimulator* simulator) {
}

void N::DoDisconnect() {
}

void N::by(const ::n::I& i) {
    // TODO
}

::n::S& N::get_value(){
    throw std::logic_error("Property getter is not implemented.");
}
void N::set_value(::n::S& value){
    // TODO
}
} // namespace n
