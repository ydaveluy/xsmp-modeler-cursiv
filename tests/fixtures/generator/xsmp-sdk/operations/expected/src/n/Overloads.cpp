// -----------------------------------------------------------------------------
// File Name    : Overloads.cpp
// -----------------------------------------------------------------------------
/// @file n/Overloads.cpp

#include <n/Overloads.h>

namespace n
{
void Overloads::DoPublish(::Smp::IPublication* receiver) {
}

void Overloads::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void Overloads::DoConnect(::Smp::ISimulator* simulator) {
}

void Overloads::DoDisconnect() {
}

void Overloads::f(::n::Skipped skipped) {
    // TODO
}

void Overloads::f(::Smp::Int32 first) {
    // TODO
}

void Overloads::f(::Smp::Float64 second) {
    // TODO
}
} // namespace n
