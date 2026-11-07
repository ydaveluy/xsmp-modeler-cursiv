// -----------------------------------------------------------------------------
// File Name    : MGen.cpp
// -----------------------------------------------------------------------------
/// @file n/MGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/M.h>
#include <Smp/ViewKind.h>
#include <Xsmp/ComponentHelper.h>

namespace n
{
//--------------------------- Constructor -------------------------
MGen::MGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator) { }

void MGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // WARNING: Operation attach is not invokable: parameter 'm' is not a SimpleType.

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::n::M>(this, receiver);
}

void MGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::n::M>(this, logger, linkRegistry);
}

void MGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::n::M>(this, simulator);
}

void MGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::n::M>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

const ::Smp::Uuid& MGen::GetUuid() const {
    return Uuid_M;
}
} // namespace n
