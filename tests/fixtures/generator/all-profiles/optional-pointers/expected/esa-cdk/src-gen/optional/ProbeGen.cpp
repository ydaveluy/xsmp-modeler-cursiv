// -----------------------------------------------------------------------------
// File Name    : ProbeGen.cpp
// -----------------------------------------------------------------------------
/// @file optional/ProbeGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <optional/Probe.h>
#include <Smp/ViewKind.h>

namespace optional
{
//--------------------------- Constructor -------------------------
ProbeGen::ProbeGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::esa::ecss::smp::cdk::Model(name, description, parent, simulator) {
}

/// Virtual destructor that is called by inherited classes as well.
ProbeGen::~ProbeGen() {
}

void ProbeGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::esa::ecss::smp::cdk::Model::Publish(receiver);

    // WARNING: Operation ope_input is not invokable: it has no @View attribute.

    // WARNING: Operation ope_string is not invokable: it has no @View attribute.

    // WARNING: Operation ope_output is not invokable: it has no @View attribute.

    // WARNING: Operation ope_inout is not invokable: it has no @View attribute.

    // WARNING: Operation ope_names is not invokable: it has no @View attribute.

    dynamic_cast<::optional::Probe*>(this)->DoPublish(receiver);
}

void ProbeGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Model::Configure(logger, linkRegistry);

    dynamic_cast<::optional::Probe*>(this)->DoConfigure(logger, linkRegistry);
}

void ProbeGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Model::Connect(simulator);

    dynamic_cast<::optional::Probe*>(this)->DoConnect(simulator);
}

void ProbeGen::Disconnect() {
    if (this->GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        dynamic_cast<::optional::Probe*>(this)->DoDisconnect();
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::esa::ecss::smp::cdk::Model::Disconnect();
}

const ::Smp::Uuid& ProbeGen::GetUuid() const {
    return Uuid_Probe;
}
} // namespace optional
