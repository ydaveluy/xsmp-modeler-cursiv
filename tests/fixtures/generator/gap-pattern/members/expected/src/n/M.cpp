// -----------------------------------------------------------------------------
// File Name    : M.cpp
// -----------------------------------------------------------------------------
/// @file n/M.cpp

#include <n/M.h>

namespace n
{
void M::DoPublish(::Smp::IPublication* receiver) {
}

void M::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void M::DoConnect(::Smp::ISimulator* simulator) {
}

void M::DoDisconnect() {
}

void MGen::shared() {
    // TODO
}

::Smp::Int32 MGen::plain() {
    // TODO
    return 0;
}

::n::M* MGen::self() {
    // TODO
    return nullptr;
}

void M::overridden() {
    // TODO
}

::n::M* M::fluent() {
    // TODO
    return this;
}
} // namespace n
