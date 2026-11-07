// -----------------------------------------------------------------------------
// File Name    : ProbeGen.cpp
// -----------------------------------------------------------------------------
/// @file limits/ProbeGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <limits/Probe.h>
#include <Smp/Publication/IPublishOperation.h>
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

namespace limits
{
//--------------------------- Constructor -------------------------
ProbeGen::ProbeGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator) { }

void ProbeGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // WARNING: Operation make is not invokable: constructors cannot be dynamically invoked.

    // WARNING: Operation create is not invokable: constructors cannot be dynamically invoked.

    // WARNING: Operation pointer is not invokable: pointer return values cannot be represented by Smp::AnySimple.

    // WARNING: Operation calculate is not invokable: overloaded operation names cannot be distinguished by Smp::IRequest.

    // WARNING: Operation calculate is not invokable: overloaded operation names cannot be distinguished by Smp::IRequest.

    // Publish operation normal
    auto* op_normal = receiver->PublishOperation(
        "normal", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_normal->PublishParameter(
        "value", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_normal->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::limits::Probe>(this, receiver);
}

void ProbeGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::limits::Probe>(this, logger, linkRegistry);
}

void ProbeGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::limits::Probe>(this, simulator);
}

void ProbeGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::limits::Probe>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

std::map<std::string_view, std::function<void(ProbeGen*, ::Smp::IRequest*)>> ProbeGen::_requestHandlers{
    // Handler for Operation normal
    {"normal",
    [](ProbeGen* component, ::Smp::IRequest *request) {
        auto p_value = ::Xsmp::Request::get<::Smp::Int32>(component, request, "value", ::Smp::PrimitiveTypeKind::PTK_Int32);

        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->normal(p_value));
    }},
};

void ProbeGen::Invoke(::Smp::IRequest* request) {
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

const ::Smp::Uuid& ProbeGen::GetUuid() const {
    return Uuid_Probe;
}
} // namespace limits
