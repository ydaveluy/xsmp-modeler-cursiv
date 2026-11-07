// -----------------------------------------------------------------------------
// File Name    : Child.h
// -----------------------------------------------------------------------------
/// @file limits/Child.h

#ifndef LIMITS_CHILD_H_
#define LIMITS_CHILD_H_

// Include the generated header file
#include <limits/ChildGen.h>

namespace limits
{
class Child: public ChildGen {
public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    Child(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~Child() noexcept override = default;

    private:
    ::Smp::Int32 normal(::Smp::Float64 value) override;
    ::Smp::Int32 fresh() override;
};
} // namespace limits

#endif // LIMITS_CHILD_H_
