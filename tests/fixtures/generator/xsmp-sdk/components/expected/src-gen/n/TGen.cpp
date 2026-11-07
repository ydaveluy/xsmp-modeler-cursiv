// -----------------------------------------------------------------------------
// File Name    : TGen.cpp
// -----------------------------------------------------------------------------
/// @file n/TGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/T.h>
#include <Smp/ViewKind.h>
#include <Xsmp/ComponentHelper.h>

namespace n
{
//--------------------------- Constructor -------------------------
TGen::TGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Service(name, description, parent, simulator) { }

void TGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Service::Publish(receiver);

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::n::T>(this, receiver);
}

void TGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Service::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::n::T>(this, logger, linkRegistry);
}

void TGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Service::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::n::T>(this, simulator);
}

void TGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::n::T>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Service::Disconnect();
}

const ::Smp::Uuid& TGen::GetUuid() const {
    return Uuid_T;
}
} // namespace n
