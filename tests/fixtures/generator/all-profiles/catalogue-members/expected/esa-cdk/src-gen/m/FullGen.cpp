// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : FullGen.cpp
// -----------------------------------------------------------------------------
/// @file m/FullGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <esa/ecss/smp/cdk/EntryPoint.h>
#include <esa/ecss/smp/cdk/EventSinkArg.h>
#include <esa/ecss/smp/cdk/EventSinkVoid.h>
#include <esa/ecss/smp/cdk/Request.h>
#include <m/Full.h>
#include <m/IBase.h>
#include <Smp/IPublication.h>
#include <Smp/Publication/IPublishOperation.h>
#include <Smp/ViewKind.h>

namespace m
{
//--------------------------- Constructor -------------------------
FullGen::FullGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IComposite* parent,
    ::Smp::ISimulator* simulator):
    // Base class
    ::m::AbstractOne(name, description, parent, simulator),
    // EntryPoint: main
    main{ new ::esa::ecss::smp::cdk::EntryPoint(
        "main", // Name
        "Run it.", // Description
        this,
        simulator,
        &FullGen::_main)},
    // EntryPoint: second
    second{ new ::esa::ecss::smp::cdk::EntryPoint(
        "second", // Name
        "", // Description
        this,
        simulator,
        &FullGen::_second)},
    // Event Sink: onTick
    onTick{ new ::esa::ecss::smp::cdk::EventSinkArg<::Smp::Int32>(
        "onTick", // Name
        "Sink.", // Description
        this,
        simulator,
        &FullGen::_onTick)},
    // Event Sink: onBang
    onBang{ new ::esa::ecss::smp::cdk::EventSinkVoid(
        "onBang", // Name
        "", // Description
        this,
        simulator,
        &FullGen::_onBang)},
    // Event Source: doTick
    doTick{ new ::esa::ecss::smp::cdk::EventSourceArg<::Smp::Int32>(
        "doTick", // Name
        "Source.", // Description
        this,
        simulator)},
    // Event Source: doBang
    doBang{ new ::esa::ecss::smp::cdk::EventSourceVoid(
        "doBang", // Name
        "", // Description
        this,
        simulator)},
    // Field pub
    pub {1},
    // Field prot
    prot {2},
    // Field priv
    priv {3},
    // Field in1
    in1 {4},
    // Field out1
    out1 {5},
    // Field tr
    tr {6},
    // Field forced
    forced {1.5},
    // Field failed
    failed {},
    // Field outArray
    outArray {{ 1, 2, 3}},
    // Field outPair
    outPair { 1, 2},
    // Field named
    named {"n"},
    // Field mode
    mode {::m::Mode::On},
    // Field span
    span {2000000000L},
    // Field stamp
    stamp {},
    // Field mut
    mut {1},
    // Field expert
    expert {1},
    // Field viewInt
    viewInt {1},
    // Association link
    link{ },
    // Association self
    self{ },
    // Association mutLink
    mutLink{ },
    // Association constLink
    constLink{ },
    // Container: c0
    c0 {new ::esa::ecss::smp::cdk::Container<::Smp::IComposite>(
        "c0",
        "A container.",
        this,
        simulator,
        1,
        1)},
    // Container: c1
    c1 {new ::esa::ecss::smp::cdk::Container<::Smp::IComposite>(
        "c1",
        "",
        this,
        simulator,
        0,
        1)},
    // Container: c2
    c2 {new ::esa::ecss::smp::cdk::Container<::Smp::IComposite>(
        "c2",
        "",
        this,
        simulator,
        2,
        5)},
    // Container: c3
    c3 {new ::esa::ecss::smp::cdk::Container<::Smp::IComposite>(
        "c3",
        "",
        this,
        simulator,
        3,
        3)},
    // Container: c4
    c4 {new ::esa::ecss::smp::cdk::Container<::Smp::IComposite>(
        "c4",
        "",
        this,
        simulator,
        0,
        -1)},
    // Container: c5
    c5 {new ::esa::ecss::smp::cdk::Container<::Smp::IComposite>(
        "c5",
        "",
        this,
        simulator,
        1,
        -1)},
    // Container: c6
    c6 {new ::esa::ecss::smp::cdk::Container<::Smp::IComposite>(
        "c6",
        "",
        this,
        simulator,
        0,
        1)},
    // Reference: r0
    r0 {new ::esa::ecss::smp::cdk::Reference<::m::IBase>(
        "r0",
        "A reference.",
        this,
        simulator,
        1,
        1)},
    // Reference: r1
    r1 {new ::esa::ecss::smp::cdk::Reference<::m::IBase>(
        "r1",
        "",
        this,
        simulator,
        0,
        1)},
    // Reference: r2
    r2 {new ::esa::ecss::smp::cdk::Reference<::m::IBase>(
        "r2",
        "",
        this,
        simulator,
        0,
        -1)},
    // Reference: r3
    r3 {new ::esa::ecss::smp::cdk::Reference<::m::IBase>(
        "r3",
        "",
        this,
        simulator,
        1,
        4)},
    // Reference: r4
    r4 {new ::esa::ecss::smp::cdk::Reference<::m::IBase>(
        "r4",
        "",
        this,
        simulator,
        0,
        1)} {
    // Add container c0
    this->AddContainer(c0);
    // Add container c1
    this->AddContainer(c1);
    // Add container c2
    this->AddContainer(c2);
    // Add container c3
    this->AddContainer(c3);
    // Add container c4
    this->AddContainer(c4);
    // Add container c5
    this->AddContainer(c5);
    // Add container c6
    this->AddContainer(c6);
    // Add reference r0
    this->AddReference(r0);
    // Add reference r1
    this->AddReference(r1);
    // Add reference r2
    this->AddReference(r2);
    // Add reference r3
    this->AddReference(r3);
    // Add reference r4
    this->AddReference(r4);
    if (!this->GetEntryPoint("main"))
    {
        // Use existing implementation to manage Entry Points
        this->AddEntryPoint(main);
    }
    else
    {
        Log(Smp::Services::ILogger::LMK_Error, "EntryPoint main redeclared");
    }
    if (!this->GetEntryPoint("second"))
    {
        // Use existing implementation to manage Entry Points
        this->AddEntryPoint(second);
    }
    else
    {
        Log(Smp::Services::ILogger::LMK_Error, "EntryPoint second redeclared");
    }
    // Add event sink onTick
    this->AddEventSink(onTick);
    // Add event sink onBang
    this->AddEventSink(onBang);
    // Add event source doTick
    this->AddEventSource(doTick);
    // Add event source doBang
    this->AddEventSource(doBang);
}

/// Virtual destructor that is called by inherited classes as well.
FullGen::~FullGen() {
    delete c0;
    c0 = nullptr;
    delete c1;
    c1 = nullptr;
    delete c2;
    c2 = nullptr;
    delete c3;
    c3 = nullptr;
    delete c4;
    c4 = nullptr;
    delete c5;
    c5 = nullptr;
    delete c6;
    c6 = nullptr;
    delete r0;
    r0 = nullptr;
    delete r1;
    r1 = nullptr;
    delete r2;
    r2 = nullptr;
    delete r3;
    r3 = nullptr;
    delete r4;
    r4 = nullptr;
    delete main;
    main = nullptr;
    delete second;
    second = nullptr;
    delete onTick;
    onTick = nullptr;
    delete onBang;
    onBang = nullptr;
    delete doTick;
    doTick = nullptr;
    delete doBang;
    doBang = nullptr;
}

void FullGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::m::AbstractOne::Publish(receiver);

    if (requestHandlers.empty()) {
        PopulateRequestHandlers<FullGen>(this, requestHandlers);
    }

    // Publish field pub
    receiver->PublishField(
        "pub", // Name
        "Doc of a field.", // Description
        &pub, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field prot
    receiver->PublishField(
        "prot", // Name
        "", // Description
        &prot, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field priv
    receiver->PublishField(
        "priv", // Name
        "", // Description
        &priv, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field in1
    receiver->PublishField(
        "in1", // Name
        "", // Description
        &in1, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        true, // Input
        false // Output
    );

    // Publish field out1
    receiver->PublishField(
        "out1", // Name
        "", // Description
        &out1, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        true // Output
    );

    // Publish field tr
    receiver->PublishField(
        "tr", // Name
        "", // Description
        &tr, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        false, // State
        false, // Input
        false // Output
    );

    // Publish field forced
    receiver->PublishField(
        "forced", // Name
        "", // Description
        &forced, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        true // Output
    );

    // Publish field failed
    receiver->PublishField(
        "failed", // Name
        "", // Description
        &failed, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field outArray
    receiver->PublishField(
        "outArray", // Name
        "", // Description
        &outArray, // Address
        ::m::Uuid_Three, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        true // Output
    );

    // Publish field outPair
    receiver->PublishField(
        "outPair", // Name
        "", // Description
        &outPair, // Address
        ::m::Uuid_Pair, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        true // Output
    );

    // Publish field named
    receiver->PublishField(
        "named", // Name
        "", // Description
        &named, // Address
        ::m::Uuid_Name, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field mode
    receiver->PublishField(
        "mode", // Name
        "", // Description
        &mode, // Address
        ::m::Uuid_Mode, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field span
    receiver->PublishField(
        "span", // Name
        "", // Description
        &span, // Address
        ::Smp::Uuids::Uuid_Duration, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field stamp
    receiver->PublishField(
        "stamp", // Name
        "", // Description
        &stamp, // Address
        ::Smp::Uuids::Uuid_DateTime, // Type UUID
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field shared
    receiver->PublishField(
        "shared", // Name
        "", // Description
        &shared, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field mut
    receiver->PublishField(
        "mut", // Name
        "", // Description
        &mut, // Address
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field expert
    receiver->PublishField(
        "expert", // Name
        "", // Description
        &expert, // Address
        ::Smp::ViewKind::VK_Expert, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // Publish field viewInt
    receiver->PublishField(
        "viewInt", // Name
        "", // Description
        &viewInt, // Address
        ::Smp::ViewKind::VK_Debug, // View Kind
        true, // State
        false, // Input
        false // Output
    );

    // WARNING: Operation mustImplement is not invokable: it has no @View attribute.

    // WARNING: Operation other is not invokable: it has no @View attribute.

    // WARNING: Operation noArgs is not invokable: it has no @View attribute.

    // WARNING: Operation retInt is not invokable: it has no @View attribute.

    // WARNING: Operation retBool is not invokable: it has no @View attribute.

    // WARNING: Operation retEnum is not invokable: it has no @View attribute.

    // WARNING: Operation retName is not invokable: it has no @View attribute.

    // WARNING: Operation retStruct is not invokable: its return type is not a SimpleType.

    // WARNING: Operation retArray is not invokable: its return type is not a SimpleType.

    // WARNING: Operation retIface is not invokable: pointer return values cannot be represented by Smp::AnySimple.

    // WARNING: Operation retStr8 is not invokable: it has no @View attribute.

    // WARNING: Operation retChar is not invokable: it has no @View attribute.

    // WARNING: Operation retF32 is not invokable: it has no @View attribute.

    // WARNING: Operation retU64 is not invokable: it has no @View attribute.

    // WARNING: Operation retDuration is not invokable: it has no @View attribute.

    // WARNING: Operation withIn is not invokable: it has no @View attribute.

    // WARNING: Operation withOut is not invokable: it has no @View attribute.

    // WARNING: Operation withStr8 is not invokable: it has no @View attribute.

    // WARNING: Operation withStr8Out is not invokable: it has no @View attribute.

    // WARNING: Operation withName is not invokable: it has no @View attribute.

    // WARNING: Operation withDefaultOut is not invokable: it has no @View attribute.

    // WARNING: Operation withEnum is not invokable: it has no @View attribute.

    // WARNING: Operation withStruct is not invokable: parameter 'p' is not a SimpleType.

    // WARNING: Operation withIface is not invokable: parameter 'i' is not a SimpleType.

    // WARNING: Operation withRef is not invokable: it has no @View attribute.

    // WARNING: Operation withPtr is not invokable: it has no @View attribute.

    // WARNING: Operation withConst is not invokable: it has no @View attribute.

    // WARNING: Operation statOp is not invokable: it has no @View attribute.

    // WARNING: Operation nonVirt is not invokable: it has no @View attribute.

    // WARNING: Operation constOp is not invokable: it has no @View attribute.

    // Publish operation viewed
    auto* op_viewed = receiver->PublishOperation(
        "viewed", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_viewed->PublishParameter(
        "a", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_viewed->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish operation viewedVoid
    receiver->PublishOperation(
        "viewedVoid", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );

    // WARNING: Operation debugged is not invokable: its 'a' parameter is of a String type, which is not supported for dynamic invocation.

    // WARNING: Operation viewedOut is not invokable: its 'a' out parameter is not supported for dynamic invocation (only 'in' parameters are supported).

    // WARNING: Operation addOp is not invokable: it has no @View attribute.

    // WARNING: Operation raises is not invokable: it has no @View attribute.

    // Publish Property plainProp
    receiver->PublishProperty(
        "plainProp", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property roProp
    receiver->PublishProperty(
        "roProp", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadOnly, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property woProp
    receiver->PublishProperty(
        "woProp", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_WriteOnly, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property rwProp
    receiver->PublishProperty(
        "rwProp", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property nameProp
    receiver->PublishProperty(
        "nameProp", // Name
        "", // Description
        ::m::Uuid_Name, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // WARNING: Property pairProp is not invokable: its type is not a SimpleType.

    // Publish Property modeProp
    receiver->PublishProperty(
        "modeProp", // Name
        "", // Description
        ::m::Uuid_Mode, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property attached
    receiver->PublishProperty(
        "attached", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property attachedRo
    receiver->PublishProperty(
        "attachedRo", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadOnly, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property constGet
    receiver->PublishProperty(
        "constGet", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property statProp
    receiver->PublishProperty(
        "statProp", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property nvProp
    receiver->PublishProperty(
        "nvProp", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property protProp
    receiver->PublishProperty(
        "protProp", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property privProp
    receiver->PublishProperty(
        "privProp", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // WARNING: Property ifaceProp is not invokable: pointer properties cannot be represented by Smp::AnySimple.

    // WARNING: Property ptrProp is not invokable: pointer properties cannot be represented by Smp::AnySimple.

    // Publish Property refProp
    receiver->PublishProperty(
        "refProp", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish Property constProp
    receiver->PublishProperty(
        "constProp", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        ::Smp::AccessKind::AK_ReadWrite, // Access Kind
        ::Smp::ViewKind::VK_All // View Kind
    );

    dynamic_cast<::m::Full*>(this)->DoPublish(receiver);
}

void FullGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::m::AbstractOne::Configure(logger, linkRegistry);

    dynamic_cast<::m::Full*>(this)->DoConfigure(logger, linkRegistry);
}

void FullGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::m::AbstractOne::Connect(simulator);

    dynamic_cast<::m::Full*>(this)->DoConnect(simulator);
}

void FullGen::Disconnect() {
    if (this->GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        dynamic_cast<::m::Full*>(this)->DoDisconnect();
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::m::AbstractOne::Disconnect();
}

::esa::ecss::smp::cdk::RequestContainer<FullGen>::Map FullGen::requestHandlers;

void FullGen::Invoke(::Smp::IRequest* request) {
    if (!request) {
        return;
    }
    auto it = requestHandlers.find(request->GetOperationName());
    if (it != requestHandlers.end()) {
        it->second->Execute(*this, request);
    } else {
        // pass the request down to the base class
        ::m::AbstractOne::Invoke(request);
    }
}

const ::Smp::Uuid& FullGen::GetUuid() const {
    return Uuid_Full;
}

// Field shared
::Smp::Int32 FullGen::shared{9};

::Smp::Int32 FullGen::sharedLink{};

::Smp::Int32 FullGen::get_attached(){
    return this->pub;
}
void FullGen::set_attached(::Smp::Int32 value) {
    this->pub = value;
}

::Smp::Int32 FullGen::get_attachedRo(){
    return this->pub;
}

::Smp::Int32 FullGen::get_statProp(){
    return FullGen::shared;
}
void FullGen::set_statProp(::Smp::Int32 value) {
    FullGen::shared = value;
}
} // namespace m
