// -----------------------------------------------------------------------------
// File Name    : ChildGen.cpp
// -----------------------------------------------------------------------------
/// @file limits/ChildGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <limits/Child.h>
#include <Smp/ViewKind.h>

namespace limits
{
//--------------------------- Constructor -------------------------
ChildGen::ChildGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::limits::Probe(name, description, parent, simulator) {
}

/// Virtual destructor that is called by inherited classes as well.
ChildGen::~ChildGen() {
}

void ChildGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::limits::Probe::Publish(receiver);

    // WARNING: Operation normal is not invokable: overloaded operation names cannot be distinguished by Smp::IRequest.

    // WARNING: Operation fresh is not invokable: it has no @View attribute.

    dynamic_cast<::limits::Child*>(this)->DoPublish(receiver);
}

void ChildGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::limits::Probe::Configure(logger, linkRegistry);

    dynamic_cast<::limits::Child*>(this)->DoConfigure(logger, linkRegistry);
}

void ChildGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::limits::Probe::Connect(simulator);

    dynamic_cast<::limits::Child*>(this)->DoConnect(simulator);
}

void ChildGen::Disconnect() {
    if (this->GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        dynamic_cast<::limits::Child*>(this)->DoDisconnect();
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::limits::Probe::Disconnect();
}

const ::Smp::Uuid& ChildGen::GetUuid() const {
    return Uuid_Child;
}
} // namespace limits
