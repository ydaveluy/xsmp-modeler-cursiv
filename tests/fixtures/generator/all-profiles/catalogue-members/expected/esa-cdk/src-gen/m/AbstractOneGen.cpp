// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : AbstractOneGen.cpp
// -----------------------------------------------------------------------------
/// @file m/AbstractOneGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <m/AbstractOne.h>
#include <Smp/ViewKind.h>

namespace m
{
//--------------------------- Constructor -------------------------
AbstractOneGen::AbstractOneGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::esa::ecss::smp::cdk::Model(name, description, parent, simulator),
    // Field connectedState
    connectedState {false},
    // Field valueState
    valueState {} {
}

/// Virtual destructor that is called by inherited classes as well.
AbstractOneGen::~AbstractOneGen() {
}

void AbstractOneGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::esa::ecss::smp::cdk::Model::Publish(receiver);

    // Publish field connectedState
    receiver->PublishField(
        "connectedState", // Name
        "", // Description
        &connectedState, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field valueState
    receiver->PublishField(
        "valueState", // Name
        "", // Description
        &valueState, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // WARNING: Operation mustImplement is not invokable: it has no @View attribute.

    // WARNING: Operation ping is not invokable: it has no @View attribute.

    // Publish Property connected
    receiver->PublishProperty(
        "connected", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Bool, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property ro
    receiver->PublishProperty(
        "ro", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadOnly, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property wo
    receiver->PublishProperty(
        "wo", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_WriteOnly, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    dynamic_cast<::m::AbstractOne*>(this)->DoPublish(receiver);
}

void AbstractOneGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Model::Configure(logger, linkRegistry);

    dynamic_cast<::m::AbstractOne*>(this)->DoConfigure(logger, linkRegistry);
}

void AbstractOneGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Model::Connect(simulator);

    dynamic_cast<::m::AbstractOne*>(this)->DoConnect(simulator);
}

void AbstractOneGen::Disconnect() {
    if (this->GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        dynamic_cast<::m::AbstractOne*>(this)->DoDisconnect();
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::esa::ecss::smp::cdk::Model::Disconnect();
}

const ::Smp::Uuid& AbstractOneGen::GetUuid() const {
    return Uuid_AbstractOne;
}

constexpr ::Smp::Int32 AbstractOneGen::K;

::Smp::Bool AbstractOneGen::get_connected(){
    return this->connectedState;
}
void AbstractOneGen::set_connected(::Smp::Bool value) {
    this->connectedState = value;
}

::Smp::Int32 AbstractOneGen::get_ro(){
    return this->valueState;
}

void AbstractOneGen::set_wo(::Smp::Int32 value) {
    this->valueState = value;
}
} // namespace m
