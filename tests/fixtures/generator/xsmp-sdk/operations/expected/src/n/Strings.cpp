// -----------------------------------------------------------------------------
// File Name    : Strings.cpp
// -----------------------------------------------------------------------------
/// @file n/Strings.cpp

#include <n/Strings.h>

namespace n
{
void Strings::DoPublish(::Smp::IPublication* receiver) {
}

void Strings::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void Strings::DoConnect(::Smp::ISimulator* simulator) {
}

void Strings::DoDisconnect() {
}

void Strings::inputString(::Smp::String8 value) {
    // TODO
}

void Strings::outputString(::Smp::String8* value) {
    // TODO
}

void Strings::inoutString(::Smp::String8* value) {
    // TODO
}

::Smp::String8 Strings::returnString() {
    // TODO
    return "";
}

::Smp::Bool Strings::returnBool() {
    // TODO
    return false;
}

::Smp::Bool Strings::get_enabled(){
    ::Smp::Bool ret {};
    return ret;
}
void Strings::set_enabled(::Smp::Bool value){
    // TODO
}
} // namespace n
