// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Serv.h
// -----------------------------------------------------------------------------
/// @file m/Serv.h

#ifndef M_SERV_H_
#define M_SERV_H_

// Include the generated header file
#include <m/ServGen.h>

namespace m
{
/// A service.
class Serv: public ServGen {
public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    Serv(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~Serv() noexcept override = default;

    private:
    ::Smp::Int32 get_ro() override;
    void set_wo(::Smp::Int32 value) override;
    ::Smp::Bool ping(::Smp::Int32 value) override;
    public:
    void _go() override;
    void _bang(::Smp::IObject* sender) override;
};
} // namespace m

#endif // M_SERV_H_
