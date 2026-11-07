// -----------------------------------------------------------------------------
// File Name    : NGen.cpp
// -----------------------------------------------------------------------------
/// @file n/NGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/I.h>
#include <n/N.h>
#include <Smp/ViewKind.h>
#include <Xsmp/ComponentHelper.h>

namespace n
{
//--------------------------- Constructor -------------------------
NGen::NGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator) { }

void NGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // WARNING: Operation by is not invokable: parameter 'i' is not a SimpleType.

    // WARNING: Property value is not invokable: its type is not a SimpleType.

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::n::N>(this, receiver);
}

void NGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::n::N>(this, logger, linkRegistry);
}

void NGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::n::N>(this, simulator);
}

void NGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::n::N>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

const ::Smp::Uuid& NGen::GetUuid() const {
    return Uuid_N;
}
} // namespace n
