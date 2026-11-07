// -----------------------------------------------------------------------------
// File Name    : MGen.cpp
// -----------------------------------------------------------------------------
/// @file n/MGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <esa/ecss/smp/cdk/EntryPoint.h>
#include <n/M.h>
#include <Smp/ViewKind.h>

namespace n
{
//--------------------------- Constructor -------------------------
MGen::MGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::esa::ecss::smp::cdk::Model(name, description, parent, simulator),
    // EntryPoint: step
    step{ new ::esa::ecss::smp::cdk::EntryPoint(
        "step", // Name
        "", // Description
        this,
        simulator,
        &MGen::_step)} {
    if (!this->GetEntryPoint("step"))
    {
        // Use existing implementation to manage Entry Points
        this->AddEntryPoint(step);
    }
    else
    {
        Log(Smp::Services::ILogger::LMK_Error, "EntryPoint step redeclared");
    }
}

/// Virtual destructor that is called by inherited classes as well.
MGen::~MGen() {
    delete step;
    step = nullptr;
}

void MGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::esa::ecss::smp::cdk::Model::Publish(receiver);

    dynamic_cast<::n::M*>(this)->DoPublish(receiver);
}

void MGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Model::Configure(logger, linkRegistry);

    dynamic_cast<::n::M*>(this)->DoConfigure(logger, linkRegistry);
}

void MGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::esa::ecss::smp::cdk::Model::Connect(simulator);

    dynamic_cast<::n::M*>(this)->DoConnect(simulator);
}

void MGen::Disconnect() {
    if (this->GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        dynamic_cast<::n::M*>(this)->DoDisconnect();
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::esa::ecss::smp::cdk::Model::Disconnect();
}

const ::Smp::Uuid& MGen::GetUuid() const {
    return Uuid_M;
}
} // namespace n
