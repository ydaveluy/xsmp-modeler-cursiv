// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : EmptyGen.cpp
// -----------------------------------------------------------------------------
/// @file m/EmptyGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <m/Empty.h>
#include <Smp/ViewKind.h>
#include <Xsmp/ComponentHelper.h>

namespace m
{
//--------------------------- Constructor -------------------------
EmptyGen::EmptyGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator) { }

void EmptyGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::m::Empty>(this, receiver);
}

void EmptyGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::m::Empty>(this, logger, linkRegistry);
}

void EmptyGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::m::Empty>(this, simulator);
}

void EmptyGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::m::Empty>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

const ::Smp::Uuid& EmptyGen::GetUuid() const {
    return Uuid_Empty;
}
} // namespace m
