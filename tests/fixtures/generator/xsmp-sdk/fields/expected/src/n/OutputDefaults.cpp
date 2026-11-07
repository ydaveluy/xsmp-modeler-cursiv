// -----------------------------------------------------------------------------
// File Name    : OutputDefaults.cpp
// -----------------------------------------------------------------------------
/// @file n/OutputDefaults.cpp

#include <n/OutputDefaults.h>

namespace n
{
void OutputDefaults::DoPublish(::Smp::IPublication* receiver) {
}

void OutputDefaults::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void OutputDefaults::DoConnect(::Smp::ISimulator* simulator) {
}

void OutputDefaults::DoDisconnect() {
}
} // namespace n
