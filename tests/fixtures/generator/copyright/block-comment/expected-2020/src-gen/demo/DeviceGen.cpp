// First license line
// Second license line
// -----------------------------------------------------------------------------
// File Name    : DeviceGen.cpp
// -----------------------------------------------------------------------------
/// @file demo/DeviceGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <demo/Device.h>
#include <Smp/ViewKind.h>
#include <Xsmp/ComponentHelper.h>

namespace demo
{
//--------------------------- Constructor -------------------------
DeviceGen::DeviceGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator) { }

void DeviceGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::demo::Device>(this, receiver);
}

void DeviceGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::demo::Device>(this, logger, linkRegistry);
}

void DeviceGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::demo::Device>(this, simulator);
}

void DeviceGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::demo::Device>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

const ::Smp::Uuid& DeviceGen::GetUuid() const {
    return Uuid_Device;
}
} // namespace demo
