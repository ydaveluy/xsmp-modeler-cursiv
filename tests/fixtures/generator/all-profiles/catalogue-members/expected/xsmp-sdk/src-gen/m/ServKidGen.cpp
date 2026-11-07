// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : ServKidGen.cpp
// -----------------------------------------------------------------------------
/// @file m/ServKidGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <m/ServKid.h>
#include <Smp/ViewKind.h>
#include <Xsmp/ComponentHelper.h>

namespace m
{
//--------------------------- Constructor -------------------------
ServKidGen::ServKidGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::m::Serv(name, description, parent, simulator) { }

void ServKidGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::m::Serv::Publish(receiver);

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::m::ServKid>(this, receiver);
}

void ServKidGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::m::Serv::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::m::ServKid>(this, logger, linkRegistry);
}

void ServKidGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::m::Serv::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::m::ServKid>(this, simulator);
}

void ServKidGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::m::ServKid>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::m::Serv::Disconnect();
}

const ::Smp::Uuid& ServKidGen::GetUuid() const {
    return Uuid_ServKid;
}
} // namespace m
