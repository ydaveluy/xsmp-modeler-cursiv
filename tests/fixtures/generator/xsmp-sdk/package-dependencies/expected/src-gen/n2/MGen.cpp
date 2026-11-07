// A field of `other` and an operation of `types` chain their packages. An
// interface has no implementation, so that `interfaces` is not chained.
// -----------------------------------------------------------------------------
// File Name    : MGen.cpp
// -----------------------------------------------------------------------------
/// @file n2/MGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n2/M.h>
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

namespace n2
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

    // Publish operation f
    auto* op_f = receiver->PublishOperation(
        "f", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_f->PublishParameter(
        "x", // Name
        "", // Description
        ::t::Uuid_T, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_f->PublishParameter(
        "return", // Name
        "", // Description
        ::t::Uuid_T, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::n2::M>(this, receiver);
}

void MGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::n2::M>(this, logger, linkRegistry);
}

void MGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::n2::M>(this, simulator);
}

void MGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::n2::M>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

std::map<std::string_view, std::function<void(MGen*, ::Smp::IRequest*)>> MGen::_requestHandlers{
    // Handler for Operation f
    {"f",
    [](MGen* component, ::Smp::IRequest *request) {
        auto p_x = ::Xsmp::Request::get<::t::T>(component, request, "x", ::Smp::PrimitiveTypeKind::PTK_Int32);

        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->f(p_x));
    }},
};

void MGen::Invoke(::Smp::IRequest* request) {
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

const ::Smp::Uuid& MGen::GetUuid() const {
    return Uuid_M;
}
} // namespace n2
