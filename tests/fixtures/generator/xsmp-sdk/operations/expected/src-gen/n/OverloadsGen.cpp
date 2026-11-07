// -----------------------------------------------------------------------------
// File Name    : OverloadsGen.cpp
// -----------------------------------------------------------------------------
/// @file n/OverloadsGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Overloads.h>
#include <Smp/ViewKind.h>
#include <Xsmp/ComponentHelper.h>

namespace n
{
//--------------------------- Constructor -------------------------
OverloadsGen::OverloadsGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator) { }

void OverloadsGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // WARNING: Operation f is not invokable: overloaded operation names cannot be distinguished by Smp::IRequest.

    // WARNING: Operation f is not invokable: overloaded operation names cannot be distinguished by Smp::IRequest.

    // WARNING: Operation f is not invokable: overloaded operation names cannot be distinguished by Smp::IRequest.

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::n::Overloads>(this, receiver);
}

void OverloadsGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::n::Overloads>(this, logger, linkRegistry);
}

void OverloadsGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::n::Overloads>(this, simulator);
}

void OverloadsGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::n::Overloads>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

const ::Smp::Uuid& OverloadsGen::GetUuid() const {
    return Uuid_Overloads;
}
} // namespace n
