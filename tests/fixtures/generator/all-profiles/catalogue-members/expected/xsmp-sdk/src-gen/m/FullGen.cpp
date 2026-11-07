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
    // EntryPoint main
    main{
        "main", // Name
        "Run it.", // Description
        this, // Parent
        [this]{this->_main();} // Callback
    },
    // EntryPoint second
    second{
        "second", // Name
        "", // Description
        this, // Parent
        [this]{this->_second();} // Callback
    },
    // EventSink onTick
    onTick{
        "onTick", // Name
        "Sink.", // Description
        this, // Parent
        [this](::Smp::IObject *sender, ::Smp::Int32 value) {this->_onTick(sender, value);}, // Callback
        ::Smp::PrimitiveTypeKind::PTK_Int32 // Primitive Type Kind
    },
    // EventSink onBang
    onBang{
        "onBang", // Name
        "", // Description
        this, // Parent
        [this](::Smp::IObject *sender) {this->_onBang(sender);} // Callback
    },
    // EventSource doTick
    doTick{
        "doTick", // Name
        "Source.", // Description
        this, // Parent
        ::Smp::PrimitiveTypeKind::PTK_Int32 // Primitive Type Kind
    },
    // EventSource doBang
    doBang{
        "doBang", // Name
        "", // Description
        this // Parent
    },
    // Field pub
    pub {1},
    // Field prot
    prot {2},
    // Field priv
    priv {3},
    // Field in1
    in1 {4},
    // Field out1
    out1{
        simulator->GetTypeRegistry(), // Type Registry
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        "out1", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        5 // Default value
    },
    // Field tr
    tr {6},
    // Field forced
    forced{
        simulator->GetTypeRegistry(), // Type Registry
        ::Smp::Uuids::Uuid_Float64, // Type UUID
        "forced", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        1.5 // Default value
    },
    // Field failed
    failed{
        simulator->GetTypeRegistry(), // Type Registry
        ::Smp::Uuids::Uuid_Bool, // Type UUID
        "failed", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All // View Kind
    },
    // Field outArray
    outArray{
        simulator->GetTypeRegistry(), // Type Registry
        ::m::Uuid_Three, // Type UUID
        "outArray", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
        {{ 1, 2, 3}} // Default value
    },
    // Field outPair
    outPair{
        simulator->GetTypeRegistry(), // Type Registry
        ::m::Uuid_Pair, // Type UUID
        "outPair", // Name
        "", // Description
        this, // Parent
        ::Smp::ViewKind::VK_All, // View Kind
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
    // Container c0
    c0 {
        "c0", // Name
        "A container.", // Description
        this, // Parent
        1, // Lower bound
        1 // Upper bound
    },
    // Container c1
    c1 {
        "c1", // Name
        "", // Description
        this, // Parent
        0, // Lower bound
        1 // Upper bound
    },
    // Container c2
    c2 {
        "c2", // Name
        "", // Description
        this, // Parent
        2, // Lower bound
        5 // Upper bound
    },
    // Container c3
    c3 {
        "c3", // Name
        "", // Description
        this, // Parent
        3, // Lower bound
        3 // Upper bound
    },
    // Container c4
    c4 {
        "c4", // Name
        "", // Description
        this, // Parent
        0, // Lower bound
        -1 // Upper bound
    },
    // Container c5
    c5 {
        "c5", // Name
        "", // Description
        this, // Parent
        1, // Lower bound
        -1 // Upper bound
    },
    // Container c6
    c6 {
        "c6", // Name
        "", // Description
        this, // Parent
        0, // Lower bound
        1 // Upper bound
    },
    // Reference r0
    r0{
        "r0", // Name
        "A reference.", // Description
        this, // Parent
        1, // Lower bound
        1 // Upper bound
    },
    // Reference r1
    r1{
        "r1", // Name
        "", // Description
        this, // Parent
        0, // Lower bound
        1 // Upper bound
    },
    // Reference r2
    r2{
        "r2", // Name
        "", // Description
        this, // Parent
        0, // Lower bound
        -1 // Upper bound
    },
    // Reference r3
    r3{
        "r3", // Name
        "", // Description
        this, // Parent
        1, // Lower bound
        4 // Upper bound
    },
    // Reference r4
    r4{
        "r4", // Name
        "", // Description
        this, // Parent
        0, // Lower bound
        1 // Upper bound
    } { }

void FullGen::Publish(::Smp::IPublication* receiver) {
    // Call parent class implementation first
    ::m::AbstractOne::Publish(receiver);

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

    // WARNING: Operation retStr8 is not invokable: its return type is Smp.String8 (its memory ownership cannot be determined, so it cannot be safely deallocated).

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

    // WARNING: Operation withStr8Out is not invokable: its 's' out parameter is of type Smp.String8 (its memory ownership cannot be determined, so it cannot be safely deallocated).

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

    // Call user DoPublish if any
    ::Xsmp::Component::Helper::Publish<::m::Full>(this, receiver);
}

void FullGen::Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) {
    // Call parent implementation first
    ::m::AbstractOne::Configure(logger, linkRegistry);

    // Call user DoConfigure if any
    ::Xsmp::Component::Helper::Configure<::m::Full>(this, logger, linkRegistry);
}

void FullGen::Connect(::Smp::ISimulator* simulator) {
    // Call parent implementation first
    ::m::AbstractOne::Connect(simulator);

    // Call user DoConnect if any
    ::Xsmp::Component::Helper::Connect<::m::Full>(this, simulator);
}

void FullGen::Disconnect() {
    if (this->::Xsmp::Component::GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
        // Call user DoDisconnect if any
        ::Xsmp::Component::Helper::Disconnect<::m::Full>(this);
    }

    // Call parent implementation last, to remove references to the Simulator and its services
    ::m::AbstractOne::Disconnect();
}

std::map<std::string_view, std::function<void(FullGen*, ::Smp::IRequest*)>> FullGen::_requestHandlers{
    // Handler for Operation mustImplement
    {"mustImplement",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->mustImplement());
    }},

    // Handler for Operation other
    {"other",
    [](FullGen* component, ::Smp::IRequest *) {
        component->other();
    }},

    // Handler for Operation noArgs
    {"noArgs",
    [](FullGen* component, ::Smp::IRequest *) {
        component->noArgs();
    }},

    // Handler for Operation retInt
    {"retInt",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->retInt());
    }},

    // Handler for Operation retBool
    {"retBool",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Bool, component->retBool());
    }},

    // Handler for Operation retEnum
    {"retEnum",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->retEnum());
    }},

    // Handler for Operation retName
    {"retName",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_String8, component->retName());
    }},

    // Handler for Operation retChar
    {"retChar",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Bool, component->retChar());
    }},

    // Handler for Operation retF32
    {"retF32",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Float32, component->retF32());
    }},

    // Handler for Operation retU64
    {"retU64",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_UInt64, component->retU64());
    }},

    // Handler for Operation retDuration
    {"retDuration",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Duration, component->retDuration());
    }},

    // Handler for Operation withIn
    {"withIn",
    [](FullGen* component, ::Smp::IRequest *request) {
        auto p_a = ::Xsmp::Request::get<::Smp::Int32>(component, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32);
        auto p_b = ::Xsmp::Request::get<::Smp::Float64>(component, request, "b", ::Smp::PrimitiveTypeKind::PTK_Float64, 2.5);
        auto p_c = ::Xsmp::Request::get<::Smp::Bool>(component, request, "c", ::Smp::PrimitiveTypeKind::PTK_Bool, true);

        component->withIn(p_a, p_b, p_c);
    }},

    // Handler for Operation withOut
    {"withOut",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Smp::Int32 p_a{};
        auto p_b = ::Xsmp::Request::get<::Smp::Float64>(component, request, "b", ::Smp::PrimitiveTypeKind::PTK_Float64);

        component->withOut(&p_a, &p_b);
        ::Xsmp::Request::set(component, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32, p_a);
        ::Xsmp::Request::set(component, request, "b", ::Smp::PrimitiveTypeKind::PTK_Float64, p_b);
    }},

    // Handler for Operation withStr8
    {"withStr8",
    [](FullGen* component, ::Smp::IRequest *request) {
        auto p_s = request->GetParameterValue(request->GetParameterIndex("s"));

        component->withStr8(static_cast<::Smp::String8>(p_s));
    }},

    // Handler for Operation withName
    {"withName",
    [](FullGen* component, ::Smp::IRequest *request) {
        auto p_n = ::Xsmp::Request::get<::m::Name>(component, request, "n", ::Smp::PrimitiveTypeKind::PTK_String8);
        ::m::Name p_o{};

        component->withName(p_n, &p_o);
        ::Xsmp::Request::set(component, request, "o", ::Smp::PrimitiveTypeKind::PTK_String8, p_o);
    }},

    // Handler for Operation withDefaultOut
    {"withDefaultOut",
    [](FullGen* component, ::Smp::IRequest *request) {
        const bool has_a = request->GetParameterIndex("a") >= 0;
        ::Smp::Int32 p_a{};

        component->withDefaultOut(has_a ? &p_a : nullptr);
        if (has_a) { ::Xsmp::Request::set(component, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32, p_a); }
    }},

    // Handler for Operation withEnum
    {"withEnum",
    [](FullGen* component, ::Smp::IRequest *request) {
        auto p_a = ::Xsmp::Request::get<::m::Mode>(component, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32, ::m::Mode::On);

        component->withEnum(p_a);
    }},

    // Handler for Operation withRef
    {"withRef",
    [](FullGen* component, ::Smp::IRequest *request) {
        auto p_i = ::Xsmp::Request::get<::Smp::Int32>(component, request, "i", ::Smp::PrimitiveTypeKind::PTK_Int32);

        component->withRef(p_i);
    }},

    // Handler for Operation withPtr
    {"withPtr",
    [](FullGen* component, ::Smp::IRequest *request) {
        auto p_i = ::Xsmp::Request::get<::Smp::Int32>(component, request, "i", ::Smp::PrimitiveTypeKind::PTK_Int32);

        component->withPtr(&p_i);
    }},

    // Handler for Operation withConst
    {"withConst",
    [](FullGen* component, ::Smp::IRequest *request) {
        auto p_i = ::Xsmp::Request::get<::Smp::Int32>(component, request, "i", ::Smp::PrimitiveTypeKind::PTK_Int32);

        component->withConst(p_i);
    }},

    // Handler for Operation statOp
    {"statOp",
    [](FullGen* component, ::Smp::IRequest *request) {
        auto p_a = ::Xsmp::Request::get<::Smp::Int32>(component, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32);

        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->statOp(p_a));
    }},

    // Handler for Operation nonVirt
    {"nonVirt",
    [](FullGen* component, ::Smp::IRequest *request) {
        auto p_a = ::Xsmp::Request::get<::Smp::Int32>(component, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32);

        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->nonVirt(p_a));
    }},

    // Handler for Operation constOp
    {"constOp",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->constOp());
    }},

    // Handler for Operation viewed
    {"viewed",
    [](FullGen* component, ::Smp::IRequest *request) {
        auto p_a = ::Xsmp::Request::get<::Smp::Int32>(component, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32);

        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->viewed(p_a));
    }},

    // Handler for Operation viewedVoid
    {"viewedVoid",
    [](FullGen* component, ::Smp::IRequest *) {
        component->viewedVoid();
    }},

    // Handler for Operation debugged
    {"debugged",
    [](FullGen* component, ::Smp::IRequest *request) {
        auto p_a = ::Xsmp::Request::get<::m::Name>(component, request, "a", ::Smp::PrimitiveTypeKind::PTK_String8);

        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->debugged(p_a));
    }},

    // Handler for Operation viewedOut
    {"viewedOut",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Smp::Int32 p_a{};

        component->viewedOut(&p_a);
        ::Xsmp::Request::set(component, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32, p_a);
    }},

    // Handler for Operation addOp
    {"addOp",
    [](FullGen* component, ::Smp::IRequest *request) {
        auto p_a = ::Xsmp::Request::get<::Smp::Int32>(component, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32);

        component->operator+=(p_a);
    }},

    // Handler for Operation raises
    {"raises",
    [](FullGen* component, ::Smp::IRequest *request) {
        auto p_a = ::Xsmp::Request::get<::Smp::Int32>(component, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32);

        component->raises(p_a);
    }},

    // Getter handler for Property plainProp
    {"get_plainProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_plainProp());
    }},

    // Setter handler for Property plainProp
    {"set_plainProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        component->set_plainProp(::Xsmp::Request::get<::Smp::Int32>(component, request, "plainProp", ::Smp::PrimitiveTypeKind::PTK_Int32));
    }},

    // Getter handler for Property roProp
    {"get_roProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_roProp());
    }},

    // Setter handler for Property woProp
    {"set_woProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        component->set_woProp(::Xsmp::Request::get<::Smp::Int32>(component, request, "woProp", ::Smp::PrimitiveTypeKind::PTK_Int32));
    }},

    // Getter handler for Property rwProp
    {"get_rwProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_rwProp());
    }},

    // Setter handler for Property rwProp
    {"set_rwProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        component->set_rwProp(::Xsmp::Request::get<::Smp::Int32>(component, request, "rwProp", ::Smp::PrimitiveTypeKind::PTK_Int32));
    }},

    // Getter handler for Property nameProp
    {"get_nameProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_String8, component->get_nameProp());
    }},

    // Setter handler for Property nameProp
    {"set_nameProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        component->set_nameProp(::Xsmp::Request::get<::m::Name>(component, request, "nameProp", ::Smp::PrimitiveTypeKind::PTK_String8));
    }},

    // Getter handler for Property modeProp
    {"get_modeProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_modeProp());
    }},

    // Setter handler for Property modeProp
    {"set_modeProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        component->set_modeProp(::Xsmp::Request::get<::m::Mode>(component, request, "modeProp", ::Smp::PrimitiveTypeKind::PTK_Int32));
    }},

    // Getter handler for Property attached
    {"get_attached",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_attached());
    }},

    // Setter handler for Property attached
    {"set_attached",
    [](FullGen* component, ::Smp::IRequest *request) {
        component->set_attached(::Xsmp::Request::get<::Smp::Int32>(component, request, "attached", ::Smp::PrimitiveTypeKind::PTK_Int32));
    }},

    // Getter handler for Property attachedRo
    {"get_attachedRo",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_attachedRo());
    }},

    // Getter handler for Property constGet
    {"get_constGet",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_constGet());
    }},

    // Setter handler for Property constGet
    {"set_constGet",
    [](FullGen* component, ::Smp::IRequest *request) {
        component->set_constGet(::Xsmp::Request::get<::Smp::Int32>(component, request, "constGet", ::Smp::PrimitiveTypeKind::PTK_Int32));
    }},

    // Getter handler for Property statProp
    {"get_statProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_statProp());
    }},

    // Setter handler for Property statProp
    {"set_statProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        component->set_statProp(::Xsmp::Request::get<::Smp::Int32>(component, request, "statProp", ::Smp::PrimitiveTypeKind::PTK_Int32));
    }},

    // Getter handler for Property nvProp
    {"get_nvProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_nvProp());
    }},

    // Setter handler for Property nvProp
    {"set_nvProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        component->set_nvProp(::Xsmp::Request::get<::Smp::Int32>(component, request, "nvProp", ::Smp::PrimitiveTypeKind::PTK_Int32));
    }},

    // Getter handler for Property protProp
    {"get_protProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_protProp());
    }},

    // Setter handler for Property protProp
    {"set_protProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        component->set_protProp(::Xsmp::Request::get<::Smp::Int32>(component, request, "protProp", ::Smp::PrimitiveTypeKind::PTK_Int32));
    }},

    // Getter handler for Property privProp
    {"get_privProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_privProp());
    }},

    // Setter handler for Property privProp
    {"set_privProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        component->set_privProp(::Xsmp::Request::get<::Smp::Int32>(component, request, "privProp", ::Smp::PrimitiveTypeKind::PTK_Int32));
    }},

    // Getter handler for Property refProp
    {"get_refProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_refProp());
    }},

    // Setter handler for Property refProp
    {"set_refProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        auto value = ::Xsmp::Request::get<::Smp::Int32>(component, request, "refProp", ::Smp::PrimitiveTypeKind::PTK_Int32);
        component->set_refProp(value);
    }},

    // Getter handler for Property constProp
    {"get_constProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        ::Xsmp::Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, component->get_constProp());
    }},

    // Setter handler for Property constProp
    {"set_constProp",
    [](FullGen* component, ::Smp::IRequest *request) {
        component->set_constProp(::Xsmp::Request::get<::Smp::Int32>(component, request, "constProp", ::Smp::PrimitiveTypeKind::PTK_Int32));
    }},
};

void FullGen::Invoke(::Smp::IRequest* request) {
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
