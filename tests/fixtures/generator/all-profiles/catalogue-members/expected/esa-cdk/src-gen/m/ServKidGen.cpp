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

namespace m
{
//--------------------------- Constructor -------------------------
ServKidGen::ServKidGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::m::Serv(name, description, parent, simulator) {
}

/// Virtual destructor that is called by inherited classes as well.
ServKidGen::~ServKidGen() {
}

void ServKidGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::m::Serv::Publish(receiver);

    dynamic_cast<::m::ServKid*>(this)->DoPublish(receiver);
}

void ServKidGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::m::Serv::Configure(logger, linkRegistry);

    dynamic_cast<::m::ServKid*>(this)->DoConfigure(logger, linkRegistry);
}

void ServKidGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::m::Serv::Connect(simulator);

    dynamic_cast<::m::ServKid*>(this)->DoConnect(simulator);
}

void ServKidGen::Disconnect() {
    if (this->GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        dynamic_cast<::m::ServKid*>(this)->DoDisconnect();
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::m::Serv::Disconnect();
}

const ::Smp::Uuid& ServKidGen::GetUuid() const {
    return Uuid_ServKid;
}
} // namespace m
