// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : OnlyOutput.cpp
// -----------------------------------------------------------------------------
/// @file m/OnlyOutput.cpp

#include <m/OnlyOutput.h>

namespace m
{
void OnlyOutput::DoPublish(::Smp::IPublication* receiver) {
}

void OnlyOutput::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void OnlyOutput::DoConnect(::Smp::ISimulator* simulator) {
}

void OnlyOutput::DoDisconnect() {
}
} // namespace m
