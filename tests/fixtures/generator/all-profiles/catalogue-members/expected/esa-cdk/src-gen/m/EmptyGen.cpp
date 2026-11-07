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

namespace m
{
//--------------------------- Constructor -------------------------
EmptyGen::EmptyGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::esa::ecss::smp::cdk::Model(name, description, parent, simulator) {
}

/// Virtual destructor that is called by inherited classes as well.
EmptyGen::~EmptyGen() {
}

void EmptyGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::esa::ecss::smp::cdk::Model::Publish(receiver);

    dynamic_cast<::m::Empty*>(this)->DoPublish(receiver);
}

void EmptyGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Model::Configure(logger, linkRegistry);

    dynamic_cast<::m::Empty*>(this)->DoConfigure(logger, linkRegistry);
}

void EmptyGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Model::Connect(simulator);

    dynamic_cast<::m::Empty*>(this)->DoConnect(simulator);
}

void EmptyGen::Disconnect() {
    if (this->GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        dynamic_cast<::m::Empty*>(this)->DoDisconnect();
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::esa::ecss::smp::cdk::Model::Disconnect();
}

const ::Smp::Uuid& EmptyGen::GetUuid() const {
    return Uuid_Empty;
}
} // namespace m
