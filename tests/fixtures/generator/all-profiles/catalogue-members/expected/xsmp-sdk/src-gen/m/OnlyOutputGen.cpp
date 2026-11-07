// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : OnlyOutputGen.cpp
// -----------------------------------------------------------------------------
/// @file m/OnlyOutputGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <m/OnlyOutput.h>
#include <Smp/ViewKind.h>
#include <Xsmp/ComponentHelper.h>

namespace m
{
//--------------------------- Constructor -------------------------
OnlyOutputGen::OnlyOutputGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator),
    // Field o
    o{
        simulator->GetTypeRegistry(), // Type Registry
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        "o", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All // View Kind
    },
    // Field f
    f{
        simulator->GetTypeRegistry(), // Type Registry
        ::Smp::Uuids::Uuid_Bool, // Type UUID
        "f", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All // View Kind
    } { }

void OnlyOutputGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // Publish field o
    receiver->PublishField(&o);

    // Publish field f
    receiver->PublishField(&f);

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::m::OnlyOutput>(this, receiver);
}

void OnlyOutputGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::m::OnlyOutput>(this, logger, linkRegistry);
}

void OnlyOutputGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::m::OnlyOutput>(this, simulator);
}

void OnlyOutputGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::m::OnlyOutput>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

const ::Smp::Uuid& OnlyOutputGen::GetUuid() const {
    return Uuid_OnlyOutput;
}
} // namespace m
