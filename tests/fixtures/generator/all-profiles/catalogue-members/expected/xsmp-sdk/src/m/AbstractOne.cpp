// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : AbstractOne.cpp
// -----------------------------------------------------------------------------
/// @file m/AbstractOne.cpp

#include <m/AbstractOne.h>

namespace m
{
void AbstractOne::DoPublish(::Smp::IPublication* receiver) {
}

void AbstractOne::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void AbstractOne::DoConnect(::Smp::ISimulator* simulator) {
}

void AbstractOne::DoDisconnect() {
}

::Smp::Bool AbstractOne::ping(::Smp::Int32 value) {
    // TODO
    return false;
}
} // namespace m
