// -----------------------------------------------------------------------------
// File Name    : Documented.cpp
// -----------------------------------------------------------------------------
/// @file n/Documented.cpp

#include <n/Documented.h>

namespace n
{
void Documented::DoPublish(::Smp::IPublication* receiver) {
}

void Documented::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void Documented::DoConnect(::Smp::ISimulator* simulator) {
}

void Documented::DoDisconnect() {
}

::Smp::Bool Documented::check(::Smp::Int32 value) {
    // TODO
    return false;
}
} // namespace n
