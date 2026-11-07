// -----------------------------------------------------------------------------
// File Name    : SGen.cpp
// -----------------------------------------------------------------------------
/// @file n/SGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/S.h>
#include <Smp/ViewKind.h>
#include <Xsmp/ComponentHelper.h>

namespace n
{
//--------------------------- Constructor -------------------------
SGen::SGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Service(name, description, parent, simulator) { }

void SGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Service::Publish(receiver);

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::n::S>(this, receiver);
}

void SGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Service::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::n::S>(this, logger, linkRegistry);
}

void SGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Service::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::n::S>(this, simulator);
}

void SGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::n::S>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Service::Disconnect();
}

const ::Smp::Uuid& SGen::GetUuid() const {
    return Uuid_S;
}
} // namespace n
