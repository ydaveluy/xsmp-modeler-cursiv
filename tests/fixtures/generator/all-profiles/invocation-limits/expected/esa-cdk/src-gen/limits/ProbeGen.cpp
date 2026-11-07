// -----------------------------------------------------------------------------
// File Name    : ProbeGen.cpp
// -----------------------------------------------------------------------------
/// @file limits/ProbeGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <limits/Probe.h>
#include <Smp/ViewKind.h>

namespace limits
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

    // WARNING: Operation make is not invokable: constructors cannot be dynamically invoked.

    // WARNING: Operation create is not invokable: constructors cannot be dynamically invoked.

    // WARNING: Operation pointer is not invokable: pointer return values cannot be represented by Smp::AnySimple.

    // WARNING: Operation calculate is not invokable: overloaded operation names cannot be distinguished by Smp::IRequest.

    // WARNING: Operation calculate is not invokable: overloaded operation names cannot be distinguished by Smp::IRequest.

    // WARNING: Operation normal is not invokable: it has no @View attribute.

    dynamic_cast<::limits::Probe*>(this)->DoPublish(receiver);
}

void ProbeGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Model::Configure(logger, linkRegistry);

    dynamic_cast<::limits::Probe*>(this)->DoConfigure(logger, linkRegistry);
}

void ProbeGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Model::Connect(simulator);

    dynamic_cast<::limits::Probe*>(this)->DoConnect(simulator);
}

void ProbeGen::Disconnect() {
    if (this->GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        dynamic_cast<::limits::Probe*>(this)->DoDisconnect();
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::esa::ecss::smp::cdk::Model::Disconnect();
}

const ::Smp::Uuid& ProbeGen::GetUuid() const {
    return Uuid_Probe;
}
} // namespace limits
