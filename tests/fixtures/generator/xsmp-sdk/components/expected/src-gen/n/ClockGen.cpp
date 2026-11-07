// -----------------------------------------------------------------------------
// File Name    : ClockGen.cpp
// -----------------------------------------------------------------------------
/// @file n/ClockGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Clock.h>
#include <Smp/ViewKind.h>
#include <Xsmp/ComponentHelper.h>

namespace n
{
//--------------------------- Constructor -------------------------
ClockGen::ClockGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Service(name, description, parent, simulator) { }

void ClockGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Service::Publish(receiver);

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::n::Clock>(this, receiver);
}

void ClockGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Service::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::n::Clock>(this, logger, linkRegistry);
}

void ClockGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Service::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::n::Clock>(this, simulator);
}

void ClockGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::n::Clock>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Service::Disconnect();
}

const ::Smp::Uuid& ClockGen::GetUuid() const {
    return Uuid_Clock;
}
} // namespace n
