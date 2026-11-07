// -----------------------------------------------------------------------------
// File Name    : ConstProperty.cpp
// -----------------------------------------------------------------------------
/// @file n/ConstProperty.cpp

#include <n/ConstProperty.h>

namespace n
{
void ConstProperty::DoPublish(::Smp::IPublication* receiver) {
}

void ConstProperty::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void ConstProperty::DoConnect(::Smp::ISimulator* simulator) {
}

void ConstProperty::DoDisconnect() {
}

const ::Smp::Int32 ConstProperty::get_p(){
    const ::Smp::Int32 ret {};
    return ret;
}
void ConstProperty::set_p(const ::Smp::Int32 value){
    // TODO
}
} // namespace n
