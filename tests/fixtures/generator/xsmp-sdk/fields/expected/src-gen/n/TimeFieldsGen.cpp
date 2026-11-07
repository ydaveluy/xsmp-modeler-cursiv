// -----------------------------------------------------------------------------
// File Name    : TimeFieldsGen.cpp
// -----------------------------------------------------------------------------
/// @file n/TimeFieldsGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/TimeFields.h>
#include <Smp/ViewKind.h>
#include <Xsmp/ComponentHelper.h>

namespace n
{
//--------------------------- Constructor -------------------------
TimeFieldsGen::TimeFieldsGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator),
    // Field span
    span {},
    // Field epoch
    epoch {},
    // Field count
    count {} { }

void TimeFieldsGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // Publish field span
    receiver->PublishField(
        "span", // Name
        "", // Description
        &span, // Address
        ::Smp::Uuids::Uuid_Duration, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field epoch
    receiver->PublishField(
        "epoch", // Name
        "", // Description
        &epoch, // Address
        ::Smp::Uuids::Uuid_DateTime, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field count
    receiver->PublishField(
        "count", // Name
        "", // Description
        &count, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::n::TimeFields>(this, receiver);
}

void TimeFieldsGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::n::TimeFields>(this, logger, linkRegistry);
}

void TimeFieldsGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::n::TimeFields>(this, simulator);
}

void TimeFieldsGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::n::TimeFields>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

const ::Smp::Uuid& TimeFieldsGen::GetUuid() const {
    return Uuid_TimeFields;
}
} // namespace n
