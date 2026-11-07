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

#include <m/Full.h>
#include <m/IBase.h>
#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Version.h>
#include <TasMdk/EntryPoint.h>
#include <TasMdk/EventSink.h>

namespace m
{
::TasMdk::Request::Handler<FullGen>::CollectionType FullGen::requestHandlers;

void FullGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call base implementation first
    ::m::AbstractOne::Configure(logger, linkRegistry);
    if (Configure_Callback) {
        Configure_Callback(logger, linkRegistry);
    }
}

void FullGen::Connect(::Smp::ISimulator* simulator) {
    // Call mdk implementation first
    ::m::AbstractOne::Connect(simulator);
    if (Connect_Callback) {
        Connect_Callback(simulator);
    }
}

void FullGen::Disconnect() {
    if (Disconnect_Callback) {
        Disconnect_Callback();
    }
    // Call parent implementation last, to remove references to the Simulator and its services
    ::m::AbstractOne::Disconnect();
}

FullGen::FullGen(
    ::Smp::String8 name,
    ::Smp::String8 description,
    ::Smp::IObject* parent,
    ::Smp::Publication::ITypeRegistry* type_registry)
    : // Base class initialization
    ::m::AbstractOne(name, description, parent, type_registry),
    // EntryPoint: main
    main{ new ::TasMdk::EntryPoint(
        "main", // Name
        "Run it.", // Description
        this,
        _entrypoints,
        std::bind(&FullGen::_main, this))},
    // EntryPoint: second
    second{ new ::TasMdk::EntryPoint(
        "second", // Name
        "", // Description
        this,
        _entrypoints,
        std::bind(&FullGen::_second, this))},
    // Event Sink: onTick
    onTick{ new ::TasMdk::EventSink(
        "onTick", // Name
        "Sink.", // Description
        this,
        _event_sinks,
        std::bind(&FullGen::_onTick, this, std::placeholders::_1))},
    // Event Sink: onBang
    onBang{ new ::TasMdk::EventSink(
        "onBang", // Name
        "", // Description
        this,
        _event_sinks,
        std::bind(&FullGen::_onBang, this, std::placeholders::_1))},
    // Event Source: doTick
    doTick{ new ::TasMdk::EventSource(
        "doTick", // Name
        "Source.", // Description
        this,
        _event_sources)},
    // Event Source: doBang
    doBang{ new ::TasMdk::EventSource(
        "doBang", // Name
        "", // Description
        this,
        _event_sources)},
    // Field pub
    pub {1},
    // Field prot
    prot {2},
    // Field priv
    priv {3},
    // Field in1
    in1{
        "in1", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        4 // Default value
    },
    // Field out1
    out1{
        "out1", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        5 // Default value
    },
    // Field tr
    tr {6},
    // Field forced
    forced{
        "forced", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::Smp::Uuids::Uuid_Float64, // Type UUID
        1.5 // Default value
    },
    // Field failed
    failed{
        "failed", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::Smp::Uuids::Uuid_Bool // Type UUID
    },
    // Field outArray
    outArray{
        "outArray", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::m::Uuid_Three, // Type UUID
        { 1, 2, 3} // Default value
    },
    // Field outPair
    outPair{
        "outPair", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        type_registry, // Type Registry
        ::m::Uuid_Pair, // Type UUID
        { 1, 2} // Default value
    },
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
    c0 {new ::TasMdk::Container<::Smp::IComposite>(
        "c0",
        "A container.",
        this,
        _containers,
        1,
        1)},
    // Container: c1
    c1 {new ::TasMdk::Container<::Smp::IComposite>(
        "c1",
        "",
        this,
        _containers,
        0,
        1)},
    // Container: c2
    c2 {new ::TasMdk::Container<::Smp::IComposite>(
        "c2",
        "",
        this,
        _containers,
        2,
        5)},
    // Container: c3
    c3 {new ::TasMdk::Container<::Smp::IComposite>(
        "c3",
        "",
        this,
        _containers,
        3,
        3)},
    // Container: c4
    c4 {new ::TasMdk::Container<::Smp::IComposite>(
        "c4",
        "",
        this,
        _containers,
        0,
        -1)},
    // Container: c5
    c5 {new ::TasMdk::Container<::Smp::IComposite>(
        "c5",
        "",
        this,
        _containers,
        1,
        -1)},
    // Container: c6
    c6 {new ::TasMdk::Container<::Smp::IComposite>(
        "c6",
        "",
        this,
        _containers,
        0,
        1)},
    // Reference: r0
    r0 {new ::TasMdk::Reference<::m::IBase>(
        "r0",
        "A reference.",
        this,
        _references,
        1,
        1)},
    // Reference: r1
    r1 {new ::TasMdk::Reference<::m::IBase>(
        "r1",
        "",
        this,
        _references,
        0,
        1)},
    // Reference: r2
    r2 {new ::TasMdk::Reference<::m::IBase>(
        "r2",
        "",
        this,
        _references,
        0,
        -1)},
    // Reference: r3
    r3 {new ::TasMdk::Reference<::m::IBase>(
        "r3",
        "",
        this,
        _references,
        1,
        4)},
    // Reference: r4
    r4 {new ::TasMdk::Reference<::m::IBase>(
        "r4",
        "",
        this,
        _references,
        0,
        1)}
{
}

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
    // Call base class implementation first
    ::m::AbstractOne::Publish(receiver);

    if (Publish_Callback) {
        Publish_Callback(receiver);
    }

    // Populate the request handlers (only once)
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
    receiver->PublishField(&in1);

    // Publish field out1
    receiver->PublishField(&out1);

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
    receiver->PublishField(&forced);

    // Publish field failed
    receiver->PublishField(&failed);

    // Publish field outArray
    receiver->PublishField(&outArray);

    // Publish field outPair
    receiver->PublishField(&outPair);

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

    // Publish operation mustImplement
    auto* op_mustImplement = receiver->PublishOperation(
        "mustImplement", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_mustImplement->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish operation other
    receiver->PublishOperation(
        "other", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish operation noArgs
    receiver->PublishOperation(
        "noArgs", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );

    // Publish operation retInt
    auto* op_retInt = receiver->PublishOperation(
        "retInt", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_retInt->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish operation retBool
    auto* op_retBool = receiver->PublishOperation(
        "retBool", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_retBool->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Bool, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish operation retEnum
    auto* op_retEnum = receiver->PublishOperation(
        "retEnum", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_retEnum->PublishParameter(
        "return", // Name
        "", // Description
        ::m::Uuid_Mode, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish operation retName
    auto* op_retName = receiver->PublishOperation(
        "retName", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_retName->PublishParameter(
        "return", // Name
        "", // Description
        ::m::Uuid_Name, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // WARNING: Operation retStruct is not invokable: its return type is not a SimpleType.

    // WARNING: Operation retArray is not invokable: its return type is not a SimpleType.

    // WARNING: Operation retIface is not invokable: pointer return values cannot be represented by Smp::AnySimple.

    // Publish operation retStr8
    auto* op_retStr8 = receiver->PublishOperation(
        "retStr8", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_retStr8->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_String8, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish operation retChar
    auto* op_retChar = receiver->PublishOperation(
        "retChar", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_retChar->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Bool, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish operation retF32
    auto* op_retF32 = receiver->PublishOperation(
        "retF32", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_retF32->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Float32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish operation retU64
    auto* op_retU64 = receiver->PublishOperation(
        "retU64", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_retU64->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_UInt64, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish operation retDuration
    auto* op_retDuration = receiver->PublishOperation(
        "retDuration", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_retDuration->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Duration, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish operation withIn
    auto* op_withIn = receiver->PublishOperation(
        "withIn", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_withIn->PublishParameter(
        "a", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_withIn->PublishParameter(
        "b", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Float64, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_withIn->PublishParameter(
        "c", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Bool, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );

    // Publish operation withOut
    auto* op_withOut = receiver->PublishOperation(
        "withOut", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_withOut->PublishParameter(
        "a", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Out // Parameter Direction Kind
    );
    op_withOut->PublishParameter(
        "b", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Float64, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_InOut // Parameter Direction Kind
    );

    // Publish operation withStr8
    auto* op_withStr8 = receiver->PublishOperation(
        "withStr8", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_withStr8->PublishParameter(
        "s", // Name
        "", // Description
        ::Smp::Uuids::Uuid_String8, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );

    // Publish operation withStr8Out
    auto* op_withStr8Out = receiver->PublishOperation(
        "withStr8Out", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_withStr8Out->PublishParameter(
        "s", // Name
        "", // Description
        ::Smp::Uuids::Uuid_String8, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Out // Parameter Direction Kind
    );

    // Publish operation withName
    auto* op_withName = receiver->PublishOperation(
        "withName", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_withName->PublishParameter(
        "n", // Name
        "", // Description
        ::m::Uuid_Name, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_withName->PublishParameter(
        "o", // Name
        "", // Description
        ::m::Uuid_Name, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Out // Parameter Direction Kind
    );

    // Publish operation withDefaultOut
    auto* op_withDefaultOut = receiver->PublishOperation(
        "withDefaultOut", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_withDefaultOut->PublishParameter(
        "a", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Out // Parameter Direction Kind
    );

    // Publish operation withEnum
    auto* op_withEnum = receiver->PublishOperation(
        "withEnum", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_withEnum->PublishParameter(
        "a", // Name
        "", // Description
        ::m::Uuid_Mode, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );

    // WARNING: Operation withStruct is not invokable: parameter 'p' is not a SimpleType.

    // WARNING: Operation withIface is not invokable: parameter 'i' is not a SimpleType.

    // Publish operation withRef
    auto* op_withRef = receiver->PublishOperation(
        "withRef", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_withRef->PublishParameter(
        "i", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );

    // Publish operation withPtr
    auto* op_withPtr = receiver->PublishOperation(
        "withPtr", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_withPtr->PublishParameter(
        "i", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );

    // Publish operation withConst
    auto* op_withConst = receiver->PublishOperation(
        "withConst", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_withConst->PublishParameter(
        "i", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );

    // Publish operation statOp
    auto* op_statOp = receiver->PublishOperation(
        "statOp", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_statOp->PublishParameter(
        "a", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_statOp->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish operation nonVirt
    auto* op_nonVirt = receiver->PublishOperation(
        "nonVirt", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_nonVirt->PublishParameter(
        "a", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_nonVirt->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish operation constOp
    auto* op_constOp = receiver->PublishOperation(
        "constOp", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_constOp->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

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

    // Publish operation debugged
    auto* op_debugged = receiver->PublishOperation(
        "debugged", // Name
        "", // Description
        ::Smp::ViewKind::VK_Debug // View Kind
    );
    op_debugged->PublishParameter(
        "a", // Name
        "", // Description
        ::m::Uuid_Name, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );
    op_debugged->PublishParameter(
        "return", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Return // Parameter Direction Kind
    );

    // Publish operation viewedOut
    auto* op_viewedOut = receiver->PublishOperation(
        "viewedOut", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_viewedOut->PublishParameter(
        "a", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_Out // Parameter Direction Kind
    );

    // Publish operation addOp
    auto* op_addOp = receiver->PublishOperation(
        "addOp", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_addOp->PublishParameter(
        "a", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );

    // Publish operation raises
    auto* op_raises = receiver->PublishOperation(
        "raises", // Name
        "", // Description
        ::Smp::ViewKind::VK_All // View Kind
    );
    op_raises->PublishParameter(
        "a", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        Smp::Publication::ParameterDirectionKind::PDK_In // Parameter Direction Kind
    );

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
}

void FullGen::Invoke(::Smp::IRequest* request) {
    if (request == nullptr) {
        return;
    }
    #if ECSS_SMP_VERSION < 202503L
    auto handler = requestHandlers.find(request->GetOperationName());
    #else
    auto handler = requestHandlers.find(request->GetName());
    #endif
    if (handler != requestHandlers.end()) {
        handler->second(*this, request);
    } else {
        // pass the request down to the base model
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
