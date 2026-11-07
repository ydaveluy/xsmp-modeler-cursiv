// -----------------------------------------------------------------------------
// File Name    : Sensor.cpp
// -----------------------------------------------------------------------------
/// @file n/Sensor.cpp

#include <n/Sensor.h>

namespace n
{
void Sensor::DoPublish(::Smp::IPublication* receiver) {
}

void Sensor::DoConfigure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
}

void Sensor::DoConnect(::Smp::ISimulator* simulator) {
}

void Sensor::DoDisconnect() {
}

void Sensor::doIt() {
    // TODO
}

void Sensor::_step() {
    // TODO implement EntryPoint step
}

void Sensor::_sink(::Smp::IObject* sender) {
    // TODO implement EventSink sink
}
} // namespace n
