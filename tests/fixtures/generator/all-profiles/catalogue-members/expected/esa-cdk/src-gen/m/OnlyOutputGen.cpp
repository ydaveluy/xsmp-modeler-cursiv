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

namespace m
{
//--------------------------- Constructor -------------------------
OnlyOutputGen::OnlyOutputGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::esa::ecss::smp::cdk::Model(name, description, parent, simulator),
    // Field o
    o {},
    // Field f
    f {} {
}

/// Virtual destructor that is called by inherited classes as well.
OnlyOutputGen::~OnlyOutputGen() {
}

void OnlyOutputGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::esa::ecss::smp::cdk::Model::Publish(receiver);

    // Publish field o
    receiver->PublishField(
        "o", // Name
        "", // Description
        &o, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        true // Output
    );

    // Publish field f
    receiver->PublishField(
        "f", // Name
        "", // Description
        &f, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    dynamic_cast<::m::OnlyOutput*>(this)->DoPublish(receiver);
}

void OnlyOutputGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Model::Configure(logger, linkRegistry);

    dynamic_cast<::m::OnlyOutput*>(this)->DoConfigure(logger, linkRegistry);
}

void OnlyOutputGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Model::Connect(simulator);

    dynamic_cast<::m::OnlyOutput*>(this)->DoConnect(simulator);
}

void OnlyOutputGen::Disconnect() {
    if (this->GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        dynamic_cast<::m::OnlyOutput*>(this)->DoDisconnect();
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::esa::ecss::smp::cdk::Model::Disconnect();
}

const ::Smp::Uuid& OnlyOutputGen::GetUuid() const {
    return Uuid_OnlyOutput;
}
} // namespace m
