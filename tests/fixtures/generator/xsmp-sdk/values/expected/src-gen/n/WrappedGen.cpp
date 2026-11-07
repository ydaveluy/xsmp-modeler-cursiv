// -----------------------------------------------------------------------------
// File Name    : WrappedGen.cpp
// -----------------------------------------------------------------------------
/// @file n/WrappedGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Wrapped.h>
#include <Smp/ViewKind.h>
#include <Xsmp/ComponentHelper.h>

namespace n
{
//--------------------------- Constructor -------------------------
WrappedGen::WrappedGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator),
    // Field wrapped
    wrapped {"x\004y"} { }

void WrappedGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // Publish field wrapped
    receiver->PublishField(
        "wrapped", // Name
        "", // Description
        &wrapped, // Address
        ::n::Uuid_Name, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::n::Wrapped>(this, receiver);
}

void WrappedGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::n::Wrapped>(this, logger, linkRegistry);
}

void WrappedGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::n::Wrapped>(this, simulator);
}

void WrappedGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::n::Wrapped>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

const ::Smp::Uuid& WrappedGen::GetUuid() const {
    return Uuid_Wrapped;
}
} // namespace n
