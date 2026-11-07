// -----------------------------------------------------------------------------
// File Name    : SensorGen.cpp
// -----------------------------------------------------------------------------
/// @file n/SensorGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Sensor.h>
#include <Smp/ViewKind.h>
#include <string>
#include <Xsmp/ComponentHelper.h>
#include <Xsmp/Request.h>

#if defined(__has_include)
#if __has_include(<Smp/Version.h>)
#include <Smp/Version.h>
#endif
#endif

#ifndef ECSS_SMP_VERSION
#define ECSS_SMP_VERSION 202003L
#endif

namespace n
{
//--------------------------- Constructor -------------------------
SensorGen::SensorGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator),
    // EntryPoint step
    step{
        "step", // Name
        "", // Description
        this, // Parent
        [this]{this->_step();} // Callback
    },
    // EventSink sink
    sink{
        "sink", // Name
        "", // Description
        this, // Parent
        [this](::Smp::IObject *sender) {this->_sink(sender);} // Callback
    },
    // EventSource source
    source{
        "source", // Name
        "", // Description
        this // Parent
    },
    // Field raw
    raw {},
    // Container slot
    slot {
        "slot", // Name
        "", // Description
        this, // Parent
        1, // Lower bound
        1 // Upper bound
    },
    // Reference ref
    ref{
        "ref", // Name
        "", // Description
        this, // Parent
        1, // Lower bound
        1 // Upper bound
    } { }

void SensorGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // Publish field raw
    receiver->PublishField(
        "raw", // Name
        "", // Description
        &raw, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish Property Value
    receiver->PublishProperty(
        "Value", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Float64, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish operation doIt
    receiver->PublishOperation(
        "doIt", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::n::Sensor>(this, receiver);
}

void SensorGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::n::Sensor>(this, logger, linkRegistry);
}

void SensorGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::n::Sensor>(this, simulator);
}

void SensorGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::n::Sensor>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

std::map<std::string_view, std::function<void(SensorGen*, ::Smp::IRequest*)>> SensorGen::_requestHandlers{
    // Handler for Operation doIt
    {"doIt",
    [](SensorGen* component, ::Smp::IRequest *) {
        component->doIt();
    }},

    // Getter handler for Property Value
    {"get_Value",
    [](SensorGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Float64, component->get_Value());
    }},

    // Setter handler for Property Value
    {"set_Value",
    [](SensorGen* component, ::Smp::IRequest *request) {
        component->set_Value(::Xsmp::Request::get<::Smp::Float64>(component, request, "Value", ::Smp::PrimitiveTypeKind::PTK_Float64));
    }},
};

void SensorGen::Invoke(::Smp::IRequest* request) {
    if (!request) {
        return;
    }
#if ECSS_SMP_VERSION >= 202503L
    // the request carries the bare property name and tells a getter
    // from a setter apart through its type
    const std::string key =
            (request->GetType() == ::Smp::RequestType::RT_Get ? "get_"
             : request->GetType() == ::Smp::RequestType::RT_Set ? "set_"
             : "") + std::string{request->GetName()};
#else
    // the request already names the property accessor
    const std::string_view key{request->GetOperationName()};
#endif
    if (auto it = _requestHandlers.find(key);
            it != _requestHandlers.end()) {
        it->second(this, request);
    } else {
        // pass the request down to the base class
        ::Xsmp::Model::Invoke(request);
    }
}

const ::Smp::Uuid& SensorGen::GetUuid() const {
    return Uuid_Sensor;
}
::Smp::Float64 SensorGen::get_Value(){
    return this->raw;
}
void SensorGen::set_Value(::Smp::Float64 value) {
    this->raw = value;
}
} // namespace n
