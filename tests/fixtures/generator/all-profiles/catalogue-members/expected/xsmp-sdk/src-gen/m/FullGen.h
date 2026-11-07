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
#include <map>
#include <Smp/IComposite.h>
#include <Smp/IRequest.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <string_view>
#include <Xsmp/Aggregate.h>
#include <Xsmp/Composite.h>
#include <Xsmp/Container.h>
#include <Xsmp/EntryPoint.h>
#include <Xsmp/EntryPointPublisher.h>
#include <Xsmp/EventConsumer.h>
#include <Xsmp/EventProvider.h>
#include <Xsmp/EventSink.h>
#include <Xsmp/EventSource.h>
#include <Xsmp/FallibleModel.h>
#include <Xsmp/Field.h>
#include <Xsmp/Reference.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace m
{
/// Universally unique identifier of type Full.
inline constexpr ::Smp::Uuid Uuid_Full { 0x20000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000021U };

class FullGen: public ::m::AbstractOne, public virtual ::m::IDerived, public virtual ::Xsmp::Composite, public virtual ::Xsmp::Aggregate, public virtual ::Xsmp::EventProvider, public virtual ::Xsmp::EventConsumer, public virtual ::Xsmp::EntryPointPublisher, public virtual ::Xsmp::FallibleModel
{
    friend class ::m::Full;
    public:
    /// Constructor setting name, description, parent and simulator.
    /// @param name Name of new Model instance.
    /// @param description Description of new Model instance.
    /// @param parent Parent of new Model instance.
    /// @param simulator The simulator instance.
    FullGen(::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IComposite* parent,
            ::Smp::ISimulator* simulator);
    /// deleted copy constructor
    FullGen(const FullGen&) = delete;
    /// deleted move constructor
    FullGen(FullGen&&) = delete;
    /// deleted copy assignment
    FullGen& operator=(const FullGen&) = delete;
    /// deleted move assignment
    FullGen& operator=(FullGen&&) = delete;

    /// Virtual destructor to release memory.
    ~FullGen() override = default;

    /// Request the Model to publish its fields, properties and
    /// operations against the provided publication receiver.
    /// @param   receiver Publication receiver.
    /// @throws  Smp::InvalidComponentState
    void Publish(::Smp::IPublication* receiver) override;

    /// Request the Model to perform any custom configuration. The
    /// component can create and configure other components using the field
    /// values of its published fields.
    /// @param   logger Logger service for logging of error messages during
    ///          configuration.
    /// @param   linkRegistry Reference to the link registry services, so
    ///          that the Model can register links that it creates
    ///          during configuration.
    /// @throws  Smp::InvalidComponentState
    void Configure(::Smp::Services::ILogger* logger, ::Smp::Services::ILinkRegistry* linkRegistry) override;

    /// Allow the Model to connect to the simulator and its simulation
    /// services.
    /// @param   simulator Simulation Environment that hosts the Model.
    /// @throws  Smp::InvalidComponentState
    void Connect(::Smp::ISimulator* simulator) override;

    /// Ask the Model to disconnect from the simulator and all its
    /// simulation services.
    /// @throws  Smp::InvalidComponentState
    void Disconnect() override;

    /// Get Universally Unique Identifier of the Model.
    /// @return  Universally Unique Identifier of the Model.
    const ::Smp::Uuid& GetUuid() const override;
    private:
    static std::map<std::string_view, std::function<void(FullGen*, ::Smp::IRequest*)>> _requestHandlers;

    public:
    /// Dynamically invoke an operation using a request object that has
    /// been created and filled with parameter values by the caller.
    /// @param   request Request object to invoke.
    /// @throws  Smp::InvalidOperationName
    /// @throws  Smp::InvalidParameterCount
    /// @throws  Smp::InvalidParameterType
    void Invoke(::Smp::IRequest* request) override;

    private:
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
    ::Xsmp::EntryPoint main;
    virtual void _main() = 0;
    ::Xsmp::EntryPoint second;
    virtual void _second() = 0;
    /// Sink.
    ::Xsmp::EventSink<::Smp::Int32> onTick;
    virtual void _onTick(::Smp::IObject* sender, ::Smp::Int32) = 0;
    ::Xsmp::EventSink<> onBang;
    virtual void _onBang(::Smp::IObject* sender) = 0;
    /// Source.
    ::Xsmp::EventSource<::Smp::Int32> doTick;
    ::Xsmp::EventSource<> doBang;
    /// Doc of a field.
    ::Smp::Int32 pub;
    protected:
    ::Smp::Int32 prot;
    private:
    ::Smp::Int32 priv;
    ::Smp::Int32 in1;
    ::Xsmp::Field<::Smp::Int32>::output out1;
    ::Smp::Int32 tr;
    ::Xsmp::Field<::Smp::Float64>::output::forcible forced;
    ::Xsmp::Field<::Smp::Bool>::failure failed;
    ::Xsmp::Field<::m::Three>::output outArray;
    ::Xsmp::Field<::m::Pair>::output outPair;
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
    ::Xsmp::Container<::Smp::IComposite> c0;
    ::Xsmp::Container<::Smp::IComposite> c1;
    ::Xsmp::Container<::Smp::IComposite> c2;
    ::Xsmp::Container<::Smp::IComposite> c3;
    ::Xsmp::Container<::Smp::IComposite> c4;
    ::Xsmp::Container<::Smp::IComposite> c5;
    ::Xsmp::Container<::Smp::IComposite> c6;
    /// A reference.
    ::Xsmp::Reference<::m::IBase> r0;
    ::Xsmp::Reference<::m::IBase> r1;
    ::Xsmp::Reference<::m::IBase> r2;
    ::Xsmp::Reference<::m::IBase> r3;
    ::Xsmp::Reference<::m::IBase> r4;
};
} // namespace m

#endif // M_FULLGEN_H_
