// -----------------------------------------------------------------------------
// File Name    : DeviceGen.cpp
// -----------------------------------------------------------------------------
/// @file n/DeviceGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Device.h>
#include <Smp/ViewKind.h>

namespace n
{
//--------------------------- Constructor -------------------------
DeviceGen::DeviceGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::esa::ecss::smp::cdk::Model(name, description, parent, simulator) {
}

/// Virtual destructor that is called by inherited classes as well.
DeviceGen::~DeviceGen() {
}

void DeviceGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::esa::ecss::smp::cdk::Model::Publish(receiver);

    // WARNING: Operation invisible is not invokable: it has no @View attribute.

    // WARNING: Operation measure is not invokable: its 'value' out parameter is not supported for dynamic invocation (only 'in' parameters are supported).

    dynamic_cast<::n::Device*>(this)->DoPublish(receiver);
}

void DeviceGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Model::Configure(logger, linkRegistry);

    dynamic_cast<::n::Device*>(this)->DoConfigure(logger, linkRegistry);
}

void DeviceGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Model::Connect(simulator);

    dynamic_cast<::n::Device*>(this)->DoConnect(simulator);
}

void DeviceGen::Disconnect() {
    if (this->GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        dynamic_cast<::n::Device*>(this)->DoDisconnect();
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::esa::ecss::smp::cdk::Model::Disconnect();
}

const ::Smp::Uuid& DeviceGen::GetUuid() const {
    return Uuid_Device;
}
} // namespace n
