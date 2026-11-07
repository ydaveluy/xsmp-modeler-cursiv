// -----------------------------------------------------------------------------
// File Name    : WrappingGen.cpp
// -----------------------------------------------------------------------------
/// @file n/WrappingGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Wrapping.h>
#include <Smp/ViewKind.h>
#include <Xsmp/ComponentHelper.h>

namespace n
{
//--------------------------- Constructor -------------------------
WrappingGen::WrappingGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator),
    // Field plain
    plain {},
    // Field list
    list {},
    // Field forced
    forced{
        simulator->GetTypeRegistry(), // Type Registry
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        "forced", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All // View Kind
    } { }

void WrappingGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // Publish field plain
    receiver->PublishField(
        "plain", // Name
        "", // Description
        &plain, // Address
        ::n::Uuid_WithOutput, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field list
    receiver->PublishField(
        "list", // Name
        "", // Description
        &list, // Address
        ::n::Uuid_Items, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field forced
    receiver->PublishField(&forced);

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::n::Wrapping>(this, receiver);
}

void WrappingGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::n::Wrapping>(this, logger, linkRegistry);
}

void WrappingGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::n::Wrapping>(this, simulator);
}

void WrappingGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::n::Wrapping>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

const ::Smp::Uuid& WrappingGen::GetUuid() const {
    return Uuid_Wrapping;
}
} // namespace n
