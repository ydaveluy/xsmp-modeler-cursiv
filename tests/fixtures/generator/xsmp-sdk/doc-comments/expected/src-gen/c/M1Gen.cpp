// -----------------------------------------------------------------------------
// File Name    : M1Gen.cpp
// -----------------------------------------------------------------------------
/// @file c/M1Gen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <c/M1.h>
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

namespace c
{
//--------------------------- Constructor -------------------------
M1Gen::M1Gen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::Xsmp::Model(name, description, parent, simulator),
    // Field g
    g {},
    // Field h
    h {},
    // Field i
    i {},
    // Field j
    j {},
    // Field k
    k {},
    // Field l
    l {},
    // Field n
    n {},
    // Field o
    o {},
    // Field p
    p {} { }

void M1Gen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::Xsmp::Model::Publish(receiver);

    // Publish operation f
    auto* op_f = receiver->PublishOperation(
        "f", // Name
        "Does things.", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_f->PublishParameter(
        "a", // Name
        "The first, see {@link T1}.", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_f->PublishParameter(
        "b", // Name
        "The second\n          on two lines.", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_f->PublishParameter(
        "c", // Name
        "Spaced.", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_f->PublishParameter(
        "return", // Name
        "The result, it\'s fine.", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish field g
    receiver->PublishField(
        "g", // Name
        "Trailing backslash \\\n and next line", // Description
        &g, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field h
    receiver->PublishField(
        "h", // Name
        "With a tab\tinside and unicode é ü 漢字.", // Description
        &h, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field i
    receiver->PublishField(
        "i", // Name
        "A question ??= trigraph and tab\t.", // Description
        &i, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish operation noText
    auto* op_noText = receiver->PublishOperation(
        "noText", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_noText->PublishParameter(
        "x", // Name
        "only a tag", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );

    // Publish field j
    receiver->PublishField(
        "j", // Name
        "", // Description
        &j, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field k
    receiver->PublishField(
        "k", // Name
        "", // Description
        &k, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field l
    receiver->PublishField(
        "l", // Name
        "", // Description
        &l, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field n
    receiver->PublishField(
        "n", // Name
        "", // Description
        &n, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field o
    receiver->PublishField(
        "o", // Name
        "{@uuid inline} kept", // Description
        &o, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field p
    receiver->PublishField(
        "p", // Name
        "Trailing spaces\n\n end", // Description
        &p, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::c::M1>(this, receiver);
}

void M1Gen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::Xsmp::Model::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::c::M1>(this, logger, linkRegistry);
}

void M1Gen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::Xsmp::Model::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::c::M1>(this, simulator);
}

void M1Gen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::c::M1>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::Xsmp::Model::Disconnect();
}

std::map<std::string_view, std::function<void(M1Gen*, ::Smp::IRequest*)>> M1Gen::_requestHandlers{
    // Handler for Operation f
    {"f",
    [](M1Gen* component, ::Smp::IRequest *request) {
        auto p_a = ::Xsmp::Request::get<::Smp::Int32>(component, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32);
        auto p_b = ::Xsmp::Request::get<::Smp::Int32>(component, request, "b", ::Smp::PrimitiveTypeKind::PTK_Int32);
        auto p_c = ::Xsmp::Request::get<::Smp::Int32>(component, request, "c", ::Smp::PrimitiveTypeKind::PTK_Int32);

        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->f(p_a, p_b, p_c));
    }},

    // Handler for Operation noText
    {"noText",
    [](M1Gen* component, ::Smp::IRequest *request) {
        auto p_x = ::Xsmp::Request::get<::Smp::Int32>(component, request, "x", ::Smp::PrimitiveTypeKind::PTK_Int32);

        component->noText(p_x);
    }},
};

void M1Gen::Invoke(::Smp::IRequest* request) {
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

const ::Smp::Uuid& M1Gen::GetUuid() const {
    return Uuid_M1;
}
} // namespace c
