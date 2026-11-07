// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : ServGen.cpp
// -----------------------------------------------------------------------------
/// @file m/ServGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <esa/ecss/smp/cdk/EntryPoint.h>
#include <esa/ecss/smp/cdk/EventSinkVoid.h>
#include <m/Serv.h>
#include <Smp/ViewKind.h>

namespace m
{
//--------------------------- Constructor -------------------------
ServGen::ServGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::esa::ecss::smp::cdk::Service(name, description, parent, simulator),
    // EntryPoint: go
    go{ new ::esa::ecss::smp::cdk::EntryPoint(
        "go", // Name
        "", // Description
        this,
        simulator,
        &ServGen::_go)},
    // Event Sink: bang
    bang{ new ::esa::ecss::smp::cdk::EventSinkVoid(
        "bang", // Name
        "", // Description
        this,
        simulator,
        &ServGen::_bang)},
    // Event Source: boom
    boom{ new ::esa::ecss::smp::cdk::EventSourceVoid(
        "boom", // Name
        "", // Description
        this,
        simulator)},
    // Field connectedState
    connectedState {},
    // Container: kids
    kids {new ::esa::ecss::smp::cdk::Container<::Smp::IComposite>(
        "kids",
        "",
        this,
        simulator,
        1,
        1)},
    // Reference: peer
    peer {new ::esa::ecss::smp::cdk::Reference<::m::IBase>(
        "peer",
        "",
        this,
        simulator,
        1,
        1)} {
    // Add container kids
    this->AddContainer(kids);
    // Add reference peer
    this->AddReference(peer);
    if (!this->GetEntryPoint("go"))
    {
        // Use existing implementation to manage Entry Points
        this->AddEntryPoint(go);
    }
    else
    {
        Log(Smp::Services::ILogger::LMK_Error, "EntryPoint go redeclared");
    }
    // Add event sink bang
    this->AddEventSink(bang);
    // Add event source boom
    this->AddEventSource(boom);
}

/// Virtual destructor that is called by inherited classes as well.
ServGen::~ServGen() {
    delete kids;
    kids = nullptr;
    delete peer;
    peer = nullptr;
    delete go;
    go = nullptr;
    delete bang;
    bang = nullptr;
    delete boom;
    boom = nullptr;
}

void ServGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::esa::ecss::smp::cdk::Service::Publish(receiver);

    // Publish field connectedState
    receiver->PublishField(
        "connectedState", // Name
        "", // Description
        &connectedState, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // WARNING: Operation ping is not invokable: it has no @View attribute.

    // Publish Property connected
    receiver->PublishProperty(
        "connected", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Bool, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property ro
    receiver->PublishProperty(
        "ro", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadOnly, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property wo
    receiver->PublishProperty(
        "wo", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_WriteOnly, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    dynamic_cast<::m::Serv*>(this)->DoPublish(receiver);
}

void ServGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Service::Configure(logger, linkRegistry);

    dynamic_cast<::m::Serv*>(this)->DoConfigure(logger, linkRegistry);
}

void ServGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Service::Connect(simulator);

    dynamic_cast<::m::Serv*>(this)->DoConnect(simulator);
}

void ServGen::Disconnect() {
    if (this->GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        dynamic_cast<::m::Serv*>(this)->DoDisconnect();
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::esa::ecss::smp::cdk::Service::Disconnect();
}

const ::Smp::Uuid& ServGen::GetUuid() const {
    return Uuid_Serv;
}

::Smp::Bool ServGen::get_connected(){
    return this->connectedState;
}
void ServGen::set_connected(::Smp::Bool value) {
    this->connectedState = value;
}
} // namespace m
