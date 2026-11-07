// -----------------------------------------------------------------------------
// File Name    : Defaults.cpp
// -----------------------------------------------------------------------------
/// @file n/Defaults.cpp

#include <n/Defaults.h>

namespace n
{
void Defaults::DoPublish(::Smp::IPublication* receiver) {
}

void Defaults::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void Defaults::DoConnect(::Smp::ISimulator* simulator) {
}

void Defaults::DoDisconnect() {
}

void Defaults::lost(::Smp::Int32 a, ::Smp::Int32* b) {
    // TODO
}

void Defaults::kept(::Smp::Int32* a, ::Smp::Int32 b) {
    // TODO
}

void Defaults::pointer(::Smp::Int32* a) {
    // TODO
}
} // namespace n
