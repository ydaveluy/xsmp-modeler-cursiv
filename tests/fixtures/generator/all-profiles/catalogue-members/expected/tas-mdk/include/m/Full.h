// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Full.h
// -----------------------------------------------------------------------------
/// @file m/Full.h

#ifndef M_FULL_H_
#define M_FULL_H_

// Include the generated header file
#include <m/FullGen.h>

namespace m
{
class Full: public FullGen {
public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    Full(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~Full() noexcept override = default;

    private:
    ::Smp::Int32 get_plainProp() override;
    void set_plainProp(::Smp::Int32 value) override;
    ::Smp::Int32 get_roProp() override;
    void set_woProp(::Smp::Int32 value) override;
    ::Smp::Int32 get_rwProp() override;
    void set_rwProp(::Smp::Int32 value) override;
    ::m::Name get_nameProp() override;
    void set_nameProp(::m::Name value) override;
    ::m::Pair get_pairProp() override;
    void set_pairProp(::m::Pair value) override;
    ::m::Mode get_modeProp() override;
    void set_modeProp(::m::Mode value) override;
    ::Smp::Int32 get_constGet() const override;
    void set_constGet(::Smp::Int32 value) override;
    protected:
    ::Smp::Int32 get_protProp() override;
    void set_protProp(::Smp::Int32 value) override;
    private:
    ::Smp::Int32 get_privProp() override;
    void set_privProp(::Smp::Int32 value) override;
    ::m::IBase* get_ifaceProp() override;
    void set_ifaceProp(::m::IBase* value) override;
    ::Smp::Int32* get_ptrProp() override;
    void set_ptrProp(::Smp::Int32* value) override;
    ::Smp::Int32& get_refProp() override;
    void set_refProp(::Smp::Int32& value) override;
    const ::Smp::Int32 get_constProp() override;
    void set_constProp(const ::Smp::Int32 value) override;
    ::Smp::Int32 mustImplement() override;
    void other() override;
    void noArgs() override;
    ::Smp::Int32 retInt() override;
    ::Smp::Bool retBool() override;
    ::m::Mode retEnum() override;
    ::m::Name retName() override;
    ::m::Pair retStruct() override;
    ::m::Three retArray() override;
    ::m::IBase* retIface() override;
    ::Smp::String8 retStr8() override;
    ::Smp::Bool retChar() override;
    ::Smp::Float32 retF32() override;
    ::Smp::UInt64 retU64() override;
    ::Smp::Duration retDuration() override;
    void withIn(::Smp::Int32 a, ::Smp::Float64 b = 2.5, ::Smp::Bool c = true) override;
    void withOut(::Smp::Int32* a, ::Smp::Float64* b) override;
    void withStr8(::Smp::String8 s) override;
    void withStr8Out(::Smp::String8* s) override;
    void withName(::m::Name n, ::m::Name* o) override;
    void withDefaultOut(::Smp::Int32* a = nullptr) override;
    void withEnum(::m::Mode a = ::m::Mode::On) override;
    void withStruct(::m::Pair p) override;
    void withIface(const ::m::IBase& i) override;
    void withRef(::Smp::Int32& i) override;
    void withPtr(::Smp::Int32* i) override;
    void withConst(const ::Smp::Int32 i) override;
    ::Smp::Int32 constOp() const override;
    ::Smp::Int32 viewed(::Smp::Int32 a) override;
    void viewedVoid() override;
    ::Smp::Int32 debugged(::m::Name a) override;
    void viewedOut(::Smp::Int32* a) override;
    void operator+=(::Smp::Int32 a) override;
    void raises(::Smp::Int32 a) override;
    public:
    void _main() override;
    void _second() override;
    void _onTick(::Smp::IObject* sender, ::Smp::Int32) override;
    void _onBang(::Smp::IObject* sender) override;
};
} // namespace m

#endif // M_FULL_H_
