// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Empty.cpp
// -----------------------------------------------------------------------------
/// @file m/Empty.cpp

#include <m/Empty.h>

namespace m
{
void Empty::DoPublish(::Smp::IPublication* receiver) {
}

void Empty::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void Empty::DoConnect(::Smp::ISimulator* simulator) {
}

void Empty::DoDisconnect() {
}
} // namespace m
