// -----------------------------------------------------------------------------
// File Name    : NullDefaults.cpp
// -----------------------------------------------------------------------------
/// @file n/NullDefaults.cpp

#include <n/NullDefaults.h>

namespace n
{
void NullDefaults::DoPublish(::Smp::IPublication* receiver) {
}

void NullDefaults::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void NullDefaults::DoConnect(::Smp::ISimulator* simulator) {
}

void NullDefaults::DoDisconnect() {
}

void NullDefaults::f(::Smp::Duration* a, ::Smp::Int32* b) {
    // TODO
}

void NullDefaults::g(::Smp::Int32* c) {
    // TODO
}

void NullDefaults::h(::Smp::Int32* d, ::Smp::Int32* e) {
    // TODO
}
} // namespace n
