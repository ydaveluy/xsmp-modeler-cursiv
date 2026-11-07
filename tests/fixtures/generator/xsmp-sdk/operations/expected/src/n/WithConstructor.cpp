// -----------------------------------------------------------------------------
// File Name    : WithConstructor.cpp
// -----------------------------------------------------------------------------
/// @file n/WithConstructor.cpp

#include <n/WithConstructor.h>

namespace n
{
void WithConstructor::DoPublish(::Smp::IPublication* receiver) {
}

void WithConstructor::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void WithConstructor::DoConnect(::Smp::ISimulator* simulator) {
}

void WithConstructor::DoDisconnect() {
}

void WithConstructor::ping() {
    // TODO
}
} // namespace n
