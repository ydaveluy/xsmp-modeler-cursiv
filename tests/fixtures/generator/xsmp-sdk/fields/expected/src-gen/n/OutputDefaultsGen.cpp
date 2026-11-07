// -----------------------------------------------------------------------------
// File Name    : OutputDefaultsGen.cpp
// -----------------------------------------------------------------------------
/// @file n/OutputDefaultsGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/OutputDefaults.h>
#include <Smp/ViewKind.h>
#include <Xsmp/ComponentHelper.h>

namespace n
{
//--------------------------- Constructor -------------------------
OutputDefaultsGen::OutputDefaultsGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator),
    // Field pair
    pair{
        simulator->GetTypeRegistry(), // Type Registry
        ::n::Uuid_Pair, // Type UUID
        "pair", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        {{ { 3}, { 4}}} // Default value
    },
    // Field grid
    grid{
        simulator->GetTypeRegistry(), // Type Registry
        ::n::Uuid_Grid, // Type UUID
        "grid", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        {{ {{ 1, 2}}, {{ 3, 4}}}} // Default value
    },
    // Field single
    single{
        simulator->GetTypeRegistry(), // Type Registry
        ::n::Uuid_Single, // Type UUID
        "single", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        { 5} // Default value
    } { }

void OutputDefaultsGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // Publish field pair
    receiver->PublishField(&pair);

    // Publish field grid
    receiver->PublishField(&grid);

    // Publish field single
    receiver->PublishField(&single);

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::n::OutputDefaults>(this, receiver);
}

void OutputDefaultsGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::n::OutputDefaults>(this, logger, linkRegistry);
}

void OutputDefaultsGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::n::OutputDefaults>(this, simulator);
}

void OutputDefaultsGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::n::OutputDefaults>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

const ::Smp::Uuid& OutputDefaultsGen::GetUuid() const {
    return Uuid_OutputDefaults;
}
} // namespace n
