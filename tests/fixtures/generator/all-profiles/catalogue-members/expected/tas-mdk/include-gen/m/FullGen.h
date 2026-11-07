// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : FullGen.h
// -----------------------------------------------------------------------------
/// @file m/FullGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef M_FULLGEN_H_
#define M_FULLGEN_H_

// ----------------------------------------------------------------------------
// --------------------------- Forward Declarations ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
class Full;
} // namespace m

namespace m
{
struct Pair;
} // namespace m

namespace m
{
class IBase;
} // namespace m

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <functional>
#include <m/AbstractOne.h>
#include <m/IBase.h>
#include <m/IDerived.h>
#include <m/Mode.h>
#include <m/Name.h>
#include <m/Pair.h>
#include <m/Three.h>
#include <Smp/IComposite.h>
#include <Smp/IEntryPoint.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <TasMdk/Container.h>
#include <TasMdk/EventSource.h>
#include <TasMdk/Field.h>
#include <TasMdk/Reference_tpl.h>
#include <type_traits>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
/// Universally unique identifier of type Full.
constexpr ::Smp::Uuid Uuid_Full { 0x20000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000021U };

class FullGen: public ::m::AbstractOne, public virtual ::m::IDerived
{
    friend class ::m::Full;

    /// Get plainProp.
    /// @return Current value of property plainProp.
    virtual ::Smp::Int32 get_plainProp()=0;
    /// Set plainProp.
    /// @param value New value of property plainProp to set.
    virtual void set_plainProp(::Smp::Int32 value)=0;
    /// Get roProp.
    /// @return Current value of property roProp.
    virtual ::Smp::Int32 get_roProp()=0;
    /// Set woProp.
    /// @param value New value of property woProp to set.
    virtual void set_woProp(::Smp::Int32 value)=0;
    /// Get rwProp.
    /// @return Current value of property rwProp.
    virtual ::Smp::Int32 get_rwProp()=0;
    /// Set rwProp.
    /// @param value New value of property rwProp to set.
    virtual void set_rwProp(::Smp::Int32 value)=0;
    /// Get nameProp.
    /// @return Current value of property nameProp.
    virtual ::m::Name get_nameProp()=0;
    /// Set nameProp.
    /// @param value New value of property nameProp to set.
    virtual void set_nameProp(::m::Name value)=0;
    /// Get pairProp.
    /// @return Current value of property pairProp.
    virtual ::m::Pair get_pairProp()=0;
    /// Set pairProp.
    /// @param value New value of property pairProp to set.
    virtual void set_pairProp(::m::Pair value)=0;
    /// Get modeProp.
    /// @return Current value of property modeProp.
    virtual ::m::Mode get_modeProp()=0;
    /// Set modeProp.
    /// @param value New value of property modeProp to set.
    virtual void set_modeProp(::m::Mode value)=0;
    /// Get attached.
    /// @return Current value of property attached.
    virtual ::Smp::Int32 get_attached();
    /// Set attached.
    /// @param value New value of property attached to set.
    virtual void set_attached(::Smp::Int32 value);
    /// Get attachedRo.
    /// @return Current value of property attachedRo.
    virtual ::Smp::Int32 get_attachedRo();
    /// Get constGet.
    /// @return Current value of property constGet.
    virtual ::Smp::Int32 get_constGet() const=0;
    /// Set constGet.
    /// @param value New value of property constGet to set.
    virtual void set_constGet(::Smp::Int32 value)=0;
    /// Get statProp.
    /// @return Current value of property statProp.
    static ::Smp::Int32 get_statProp();
    /// Set statProp.
    /// @param value New value of property statProp to set.
    static void set_statProp(::Smp::Int32 value);
    /// Get nvProp.
    /// @return Current value of property nvProp.
    ::Smp::Int32 get_nvProp();
    /// Set nvProp.
    /// @param value New value of property nvProp to set.
    void set_nvProp(::Smp::Int32 value);
    protected:
    /// Get protProp.
    /// @return Current value of property protProp.
    virtual ::Smp::Int32 get_protProp()=0;
    /// Set protProp.
    /// @param value New value of property protProp to set.
    virtual void set_protProp(::Smp::Int32 value)=0;
    private:
    /// Get privProp.
    /// @return Current value of property privProp.
    virtual ::Smp::Int32 get_privProp()=0;
    /// Set privProp.
    /// @param value New value of property privProp to set.
    virtual void set_privProp(::Smp::Int32 value)=0;
    /// Get ifaceProp.
    /// @return Current value of property ifaceProp.
    virtual ::m::IBase* get_ifaceProp()=0;
    /// Set ifaceProp.
    /// @param value New value of property ifaceProp to set.
    virtual void set_ifaceProp(::m::IBase* value)=0;
    /// Get ptrProp.
    /// @return Current value of property ptrProp.
    virtual ::Smp::Int32* get_ptrProp()=0;
    /// Set ptrProp.
    /// @param value New value of property ptrProp to set.
    virtual void set_ptrProp(::Smp::Int32* value)=0;
    /// Get refProp.
    /// @return Current value of property refProp.
    virtual ::Smp::Int32& get_refProp()=0;
    /// Set refProp.
    /// @param value New value of property refProp to set.
    virtual void set_refProp(::Smp::Int32& value)=0;
    /// Get constProp.
    /// @return Current value of property constProp.
    virtual const ::Smp::Int32 get_constProp()=0;
    /// Set constProp.
    /// @param value New value of property constProp to set.
    virtual void set_constProp(const ::Smp::Int32 value)=0;
    virtual ::Smp::Int32 mustImplement()=0;
    virtual void other()=0;
    virtual void noArgs()=0;
    virtual ::Smp::Int32 retInt()=0;
    virtual ::Smp::Bool retBool()=0;
    virtual ::m::Mode retEnum()=0;
    virtual ::m::Name retName()=0;
    virtual ::m::Pair retStruct()=0;
    virtual ::m::Three retArray()=0;
    virtual ::m::IBase* retIface()=0;
    virtual ::Smp::String8 retStr8()=0;
    virtual ::Smp::Bool retChar()=0;
    virtual ::Smp::Float32 retF32()=0;
    virtual ::Smp::UInt64 retU64()=0;
    virtual ::Smp::Duration retDuration()=0;
    virtual void withIn(::Smp::Int32 a, ::Smp::Float64 b = 2.5, ::Smp::Bool c = true)=0;
    virtual void withOut(::Smp::Int32* a, ::Smp::Float64* b)=0;
    virtual void withStr8(::Smp::String8 s)=0;
    virtual void withStr8Out(::Smp::String8* s)=0;
    virtual void withName(::m::Name n, ::m::Name* o)=0;
    virtual void withDefaultOut(::Smp::Int32* a = nullptr)=0;
    virtual void withEnum(::m::Mode a = ::m::Mode::On)=0;
    virtual void withStruct(::m::Pair p)=0;
    virtual void withIface(const ::m::IBase& i)=0;
    virtual void withRef(::Smp::Int32& i)=0;
    virtual void withPtr(::Smp::Int32* i)=0;
    virtual void withConst(const ::Smp::Int32 i)=0;
    static ::Smp::Int32 statOp(::Smp::Int32 a);
    ::Smp::Int32 nonVirt(::Smp::Int32 a);
    virtual ::Smp::Int32 constOp() const=0;
    virtual ::Smp::Int32 viewed(::Smp::Int32 a)=0;
    virtual void viewedVoid()=0;
    virtual ::Smp::Int32 debugged(::m::Name a)=0;
    virtual void viewedOut(::Smp::Int32* a)=0;
    virtual void operator+=(::Smp::Int32 a)=0;
    virtual void raises(::Smp::Int32 a)=0;
    public:
    /// Run it.
    ::Smp::IEntryPoint* main;
    virtual void _main() = 0;
    ::Smp::IEntryPoint* second;
    virtual void _second() = 0;
    /// Sink.
    ::Smp::IEventSink* onTick;
    virtual void _onTick(::Smp::IObject* sender, ::Smp::Int32) = 0;
    ::Smp::IEventSink* onBang;
    virtual void _onBang(::Smp::IObject* sender) = 0;
    /// Source.
    ::TasMdk::EventSource *doTick;
    ::TasMdk::EventSource *doBang;
    /// Doc of a field.
    ::Smp::Int32 pub;
    protected:
    ::Smp::Int32 prot;
    private:
    ::Smp::Int32 priv;
    typename ::TasMdk::SimpleField<::Smp::Int32>::input::type in1;
    typename ::TasMdk::SimpleField<::Smp::Int32>::output::type out1;
    ::Smp::Int32 tr;
    typename ::TasMdk::SimpleField<::Smp::Float64>::output::forcible::type forced;
    typename ::TasMdk::SimpleField<::Smp::Bool>::failure::type failed;
    typename ::TasMdk::ArrayField<::m::_Three>::output::type outArray;
    typename ::TasMdk::StructureField<::m::_Pair>::output::type outPair;
    ::m::Name named;
    ::m::Mode mode;
    ::Smp::Duration span;
    ::Smp::DateTime stamp;
    static ::Smp::Int32 shared;
    mutable ::Smp::Int32 mut;
    ::Smp::Int32 expert;
    ::Smp::Int32 viewInt;
    /// An association.
    ::m::Pair link;
    ::m::Full* self;
    static ::Smp::Int32 sharedLink;
    mutable ::Smp::Int32 mutLink;
    const ::Smp::Int32 constLink;
    public:
    /// A container.
    ::TasMdk::Container<::Smp::IComposite>* c0;
    ::TasMdk::Container<::Smp::IComposite>* c1;
    ::TasMdk::Container<::Smp::IComposite>* c2;
    ::TasMdk::Container<::Smp::IComposite>* c3;
    ::TasMdk::Container<::Smp::IComposite>* c4;
    ::TasMdk::Container<::Smp::IComposite>* c5;
    ::TasMdk::Container<::Smp::IComposite>* c6;
    /// A reference.
    ::TasMdk::Reference<::m::IBase>* r0;
    ::TasMdk::Reference<::m::IBase>* r1;
    ::TasMdk::Reference<::m::IBase>* r2;
    ::TasMdk::Reference<::m::IBase>* r3;
    ::TasMdk::Reference<::m::IBase>* r4;

    // ------------------------------------------------------------------------------------
    // ------------------------- Constructors/Destructor -------------------------
    // ------------------------------------------------------------------------------------
    public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    FullGen(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~FullGen() override;

    static ::TasMdk::Request::Handler<FullGen>::CollectionType requestHandlers;

    // ----------------------------------------------------------------------------------
    // ------------------------------- IComponent --------------------------------
    // ----------------------------------------------------------------------------------

    /// Publish fields, operations and properties of the model.
    /// @param receiver Publication receiver.
    void Publish(::Smp::IPublication* receiver) override;

    /// Request for configuration.
    /// @param logger Logger to use for log messages during Configure().
    /// @param linkRegistry Link Registry to use for registration of
    ///         links created during Configure() or later.
    void Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) override;

    /// Connect model to simulator.
    /// @param simulator Simulation  Environment  that  hosts the model.
    ///
    void Connect(
        ::Smp::ISimulator* simulator) override;

    /// Disconnect model to simulator.
    /// @throws Smp::InvalidComponentState
    void Disconnect() override;

    /// Return the Universally Unique Identifier of the Model.
    /// @return  Universally Unique Identifier of the Model.
    const Smp::Uuid& GetUuid() const override;

    /// Invoke the operation for the given request.
    /// @param request Request object to invoke.
    void Invoke(::Smp::IRequest* request) override;

    protected:
    /// It populates the static map of request handler to implement the invoke function.
    /// @param bluePrint an object to use as blue print if needed
    /// @param handlers the map to be populated.
    template <typename _Type>
    static void PopulateRequestHandlers(_Type* bluePrint, typename ::TasMdk::Request::Handler<_Type>::CollectionType& handlers);

    private:
    /// Callback for custom publication
    std::function<void(::Smp::IPublication*)> Publish_Callback;
    /// Callback for custom configuration
    std::function<void(::Smp::Services::ILogger *, ::Smp::Services::ILinkRegistry*)> Configure_Callback;
    /// Callback for custom connection
    std::function<void(::Smp::ISimulator*)> Connect_Callback;
    /// Callback for custom disconnection
    std::function<void()> Disconnect_Callback;
};

template <typename _Type>
void FullGen::PopulateRequestHandlers(
    [[maybe_unused]] _Type* bluePrint,
    [[maybe_unused]] typename ::TasMdk::Request::Handler<_Type>::CollectionType& handlers)
{
    if (handlers.find("mustImplement") == handlers.end()) {
        handlers["mustImplement"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            /// Invoke mustImplement
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.mustImplement()});
        };
    }

    if (handlers.find("other") == handlers.end()) {
        handlers["other"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            /// Invoke other
            component.other();
        };
    }

    if (handlers.find("noArgs") == handlers.end()) {
        handlers["noArgs"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            /// Invoke noArgs
            component.noArgs();
        };
    }

    if (handlers.find("retInt") == handlers.end()) {
        handlers["retInt"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            /// Invoke retInt
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.retInt()});
        };
    }

    if (handlers.find("retBool") == handlers.end()) {
        handlers["retBool"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            /// Invoke retBool
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Bool, component.retBool()});
        };
    }

    if (handlers.find("retEnum") == handlers.end()) {
        handlers["retEnum"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            /// Invoke retEnum
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.retEnum()});
        };
    }

    if (handlers.find("retName") == handlers.end()) {
        handlers["retName"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            /// Invoke retName
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_String8, component.retName()});
        };
    }

    if (handlers.find("retStr8") == handlers.end()) {
        handlers["retStr8"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            /// Invoke retStr8
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_String8, component.retStr8()});
        };
    }

    if (handlers.find("retChar") == handlers.end()) {
        handlers["retChar"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            /// Invoke retChar
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Bool, component.retChar()});
        };
    }

    if (handlers.find("retF32") == handlers.end()) {
        handlers["retF32"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            /// Invoke retF32
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Float32, component.retF32()});
        };
    }

    if (handlers.find("retU64") == handlers.end()) {
        handlers["retU64"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            /// Invoke retU64
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_UInt64, component.retU64()});
        };
    }

    if (handlers.find("retDuration") == handlers.end()) {
        handlers["retDuration"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            /// Invoke retDuration
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Duration, component.retDuration()});
        };
    }

    if (handlers.find("withIn") == handlers.end()) {
        handlers["withIn"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            ::Smp::Int32 p_a;
            ::TasMdk::Request::initParameter(p_a, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32);
            ::Smp::Float64 p_b{2.5};
            ::TasMdk::Request::initParameter(p_b, request, "b", ::Smp::PrimitiveTypeKind::PTK_Float64, true);
            ::Smp::Bool p_c{true};
            ::TasMdk::Request::initParameter(p_c, request, "c", ::Smp::PrimitiveTypeKind::PTK_Bool, true);

            /// Invoke withIn
            component.withIn(p_a, p_b, p_c);
        };
    }

    if (handlers.find("withOut") == handlers.end()) {
        handlers["withOut"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            ::Smp::Int32 p_a;
            ::Smp::Float64 p_b;
            ::TasMdk::Request::initParameter(p_b, request, "b", ::Smp::PrimitiveTypeKind::PTK_Float64);

            /// Invoke withOut
            component.withOut(&p_a, &p_b);
            ::TasMdk::Request::setParameter(request, "a", {::Smp::PrimitiveTypeKind::PTK_Int32, p_a});
            ::TasMdk::Request::setParameter(request, "b", {::Smp::PrimitiveTypeKind::PTK_Float64, p_b});
        };
    }

    if (handlers.find("withStr8") == handlers.end()) {
        handlers["withStr8"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            ::Smp::String8 p_s;
            ::TasMdk::Request::initParameter(p_s, request, "s", ::Smp::PrimitiveTypeKind::PTK_String8);

            /// Invoke withStr8
            component.withStr8(p_s);
        };
    }

    if (handlers.find("withStr8Out") == handlers.end()) {
        handlers["withStr8Out"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            ::Smp::String8 p_s;

            /// Invoke withStr8Out
            component.withStr8Out(&p_s);
            ::TasMdk::Request::setParameter(request, "s", {::Smp::PrimitiveTypeKind::PTK_String8, p_s});
        };
    }

    if (handlers.find("withName") == handlers.end()) {
        handlers["withName"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            ::m::Name p_n;
            ::TasMdk::Request::initParameter(p_n, request, "n", ::Smp::PrimitiveTypeKind::PTK_String8);
            ::m::Name p_o;

            /// Invoke withName
            component.withName(p_n, &p_o);
            ::TasMdk::Request::setParameter(request, "o", {::Smp::PrimitiveTypeKind::PTK_String8, p_o.data()});
        };
    }

    if (handlers.find("withDefaultOut") == handlers.end()) {
        handlers["withDefaultOut"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            const bool has_a = request->GetParameterIndex("a") >= 0;
            ::Smp::Int32 p_a{};

            /// Invoke withDefaultOut
            component.withDefaultOut(has_a ? &p_a : nullptr);
            if (has_a) { ::TasMdk::Request::setParameter(request, "a", {::Smp::PrimitiveTypeKind::PTK_Int32, p_a}); }
        };
    }

    if (handlers.find("withEnum") == handlers.end()) {
        handlers["withEnum"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            ::m::Mode p_a{::m::Mode::On};
            ::TasMdk::Request::initParameter(p_a, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32, true);

            /// Invoke withEnum
            component.withEnum(p_a);
        };
    }

    if (handlers.find("withRef") == handlers.end()) {
        handlers["withRef"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            ::Smp::Int32 p_i;
            ::TasMdk::Request::initParameter(p_i, request, "i", ::Smp::PrimitiveTypeKind::PTK_Int32);

            /// Invoke withRef
            component.withRef(p_i);
        };
    }

    if (handlers.find("withPtr") == handlers.end()) {
        handlers["withPtr"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            ::Smp::Int32 p_i;
            ::TasMdk::Request::initParameter(p_i, request, "i", ::Smp::PrimitiveTypeKind::PTK_Int32);

            /// Invoke withPtr
            component.withPtr(&p_i);
        };
    }

    if (handlers.find("withConst") == handlers.end()) {
        handlers["withConst"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            ::Smp::Int32 p_i;
            ::TasMdk::Request::initParameter(p_i, request, "i", ::Smp::PrimitiveTypeKind::PTK_Int32);

            /// Invoke withConst
            component.withConst(p_i);
        };
    }

    if (handlers.find("statOp") == handlers.end()) {
        handlers["statOp"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            ::Smp::Int32 p_a;
            ::TasMdk::Request::initParameter(p_a, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32);

            /// Invoke statOp
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.statOp(p_a)});
        };
    }

    if (handlers.find("nonVirt") == handlers.end()) {
        handlers["nonVirt"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            ::Smp::Int32 p_a;
            ::TasMdk::Request::initParameter(p_a, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32);

            /// Invoke nonVirt
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.nonVirt(p_a)});
        };
    }

    if (handlers.find("constOp") == handlers.end()) {
        handlers["constOp"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            /// Invoke constOp
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.constOp()});
        };
    }

    if (handlers.find("viewed") == handlers.end()) {
        handlers["viewed"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            ::Smp::Int32 p_a;
            ::TasMdk::Request::initParameter(p_a, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32);

            /// Invoke viewed
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.viewed(p_a)});
        };
    }

    if (handlers.find("viewedVoid") == handlers.end()) {
        handlers["viewedVoid"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            /// Invoke viewedVoid
            component.viewedVoid();
        };
    }

    if (handlers.find("debugged") == handlers.end()) {
        handlers["debugged"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            ::m::Name p_a;
            ::TasMdk::Request::initParameter(p_a, request, "a", ::Smp::PrimitiveTypeKind::PTK_String8);

            /// Invoke debugged
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.debugged(p_a)});
        };
    }

    if (handlers.find("viewedOut") == handlers.end()) {
        handlers["viewedOut"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            ::Smp::Int32 p_a;

            /// Invoke viewedOut
            component.viewedOut(&p_a);
            ::TasMdk::Request::setParameter(request, "a", {::Smp::PrimitiveTypeKind::PTK_Int32, p_a});
        };
    }

    if (handlers.find("addOp") == handlers.end()) {
        handlers["addOp"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            ::Smp::Int32 p_a;
            ::TasMdk::Request::initParameter(p_a, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32);

            /// Invoke addOp
            component.operator+=(p_a);
        };
    }

    if (handlers.find("raises") == handlers.end()) {
        handlers["raises"] = [](_Type & component, [[maybe_unused]] ::Smp::IRequest* request) {
            ::Smp::Int32 p_a;
            ::TasMdk::Request::initParameter(p_a, request, "a", ::Smp::PrimitiveTypeKind::PTK_Int32);

            /// Invoke raises
            component.raises(p_a);
        };
    }

    if (handlers.find("get_plainProp") == handlers.end()) {
        handlers["get_plainProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke get_plainProp
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.get_plainProp()});
        };
    }

    if (handlers.find("set_plainProp") == handlers.end()) {
        handlers["set_plainProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke set_plainProp
            ::Smp::Int32 plainProp;
            ::TasMdk::Request::initParameter(plainProp, request, "plainProp", ::Smp::PrimitiveTypeKind::PTK_Int32);
            component.set_plainProp(plainProp);
        };
    }

    if (handlers.find("get_roProp") == handlers.end()) {
        handlers["get_roProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke get_roProp
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.get_roProp()});
        };
    }

    if (handlers.find("set_woProp") == handlers.end()) {
        handlers["set_woProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke set_woProp
            ::Smp::Int32 woProp;
            ::TasMdk::Request::initParameter(woProp, request, "woProp", ::Smp::PrimitiveTypeKind::PTK_Int32);
            component.set_woProp(woProp);
        };
    }

    if (handlers.find("get_rwProp") == handlers.end()) {
        handlers["get_rwProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke get_rwProp
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.get_rwProp()});
        };
    }

    if (handlers.find("set_rwProp") == handlers.end()) {
        handlers["set_rwProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke set_rwProp
            ::Smp::Int32 rwProp;
            ::TasMdk::Request::initParameter(rwProp, request, "rwProp", ::Smp::PrimitiveTypeKind::PTK_Int32);
            component.set_rwProp(rwProp);
        };
    }

    if (handlers.find("get_nameProp") == handlers.end()) {
        handlers["get_nameProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke get_nameProp
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_String8, component.get_nameProp()});
        };
    }

    if (handlers.find("set_nameProp") == handlers.end()) {
        handlers["set_nameProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke set_nameProp
            ::m::Name nameProp;
            ::TasMdk::Request::initParameter(nameProp, request, "nameProp", ::Smp::PrimitiveTypeKind::PTK_String8);
            component.set_nameProp(nameProp);
        };
    }

    if (handlers.find("get_modeProp") == handlers.end()) {
        handlers["get_modeProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke get_modeProp
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.get_modeProp()});
        };
    }

    if (handlers.find("set_modeProp") == handlers.end()) {
        handlers["set_modeProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke set_modeProp
            ::m::Mode modeProp;
            ::TasMdk::Request::initParameter(modeProp, request, "modeProp", ::Smp::PrimitiveTypeKind::PTK_Int32);
            component.set_modeProp(modeProp);
        };
    }

    if (handlers.find("get_attached") == handlers.end()) {
        handlers["get_attached"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke get_attached
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.get_attached()});
        };
    }

    if (handlers.find("set_attached") == handlers.end()) {
        handlers["set_attached"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke set_attached
            ::Smp::Int32 attached;
            ::TasMdk::Request::initParameter(attached, request, "attached", ::Smp::PrimitiveTypeKind::PTK_Int32);
            component.set_attached(attached);
        };
    }

    if (handlers.find("get_attachedRo") == handlers.end()) {
        handlers["get_attachedRo"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke get_attachedRo
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.get_attachedRo()});
        };
    }

    if (handlers.find("get_constGet") == handlers.end()) {
        handlers["get_constGet"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke get_constGet
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.get_constGet()});
        };
    }

    if (handlers.find("set_constGet") == handlers.end()) {
        handlers["set_constGet"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke set_constGet
            ::Smp::Int32 constGet;
            ::TasMdk::Request::initParameter(constGet, request, "constGet", ::Smp::PrimitiveTypeKind::PTK_Int32);
            component.set_constGet(constGet);
        };
    }

    if (handlers.find("get_statProp") == handlers.end()) {
        handlers["get_statProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke get_statProp
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.get_statProp()});
        };
    }

    if (handlers.find("set_statProp") == handlers.end()) {
        handlers["set_statProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke set_statProp
            ::Smp::Int32 statProp;
            ::TasMdk::Request::initParameter(statProp, request, "statProp", ::Smp::PrimitiveTypeKind::PTK_Int32);
            component.set_statProp(statProp);
        };
    }

    if (handlers.find("get_nvProp") == handlers.end()) {
        handlers["get_nvProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke get_nvProp
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.get_nvProp()});
        };
    }

    if (handlers.find("set_nvProp") == handlers.end()) {
        handlers["set_nvProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke set_nvProp
            ::Smp::Int32 nvProp;
            ::TasMdk::Request::initParameter(nvProp, request, "nvProp", ::Smp::PrimitiveTypeKind::PTK_Int32);
            component.set_nvProp(nvProp);
        };
    }

    if (handlers.find("get_protProp") == handlers.end()) {
        handlers["get_protProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke get_protProp
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.get_protProp()});
        };
    }

    if (handlers.find("set_protProp") == handlers.end()) {
        handlers["set_protProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke set_protProp
            ::Smp::Int32 protProp;
            ::TasMdk::Request::initParameter(protProp, request, "protProp", ::Smp::PrimitiveTypeKind::PTK_Int32);
            component.set_protProp(protProp);
        };
    }

    if (handlers.find("get_privProp") == handlers.end()) {
        handlers["get_privProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke get_privProp
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.get_privProp()});
        };
    }

    if (handlers.find("set_privProp") == handlers.end()) {
        handlers["set_privProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke set_privProp
            ::Smp::Int32 privProp;
            ::TasMdk::Request::initParameter(privProp, request, "privProp", ::Smp::PrimitiveTypeKind::PTK_Int32);
            component.set_privProp(privProp);
        };
    }

    if (handlers.find("get_refProp") == handlers.end()) {
        handlers["get_refProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke get_refProp
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.get_refProp()});
        };
    }

    if (handlers.find("set_refProp") == handlers.end()) {
        handlers["set_refProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke set_refProp
            ::Smp::Int32 refProp;
            ::TasMdk::Request::initParameter(refProp, request, "refProp", ::Smp::PrimitiveTypeKind::PTK_Int32);
            component.set_refProp(refProp);
        };
    }

    if (handlers.find("get_constProp") == handlers.end()) {
        handlers["get_constProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke get_constProp
            request->SetReturnValue({::Smp::PrimitiveTypeKind::PTK_Int32, component.get_constProp()});
        };
    }

    if (handlers.find("set_constProp") == handlers.end()) {
        handlers["set_constProp"] = [](_Type & component, ::Smp::IRequest* request) {
            /// Invoke set_constProp
            ::Smp::Int32 constProp;
            ::TasMdk::Request::initParameter(constProp, request, "constProp", ::Smp::PrimitiveTypeKind::PTK_Int32);
            component.set_constProp(constProp);
        };
    }
}
} // namespace m

#endif // M_FULLGEN_H_
