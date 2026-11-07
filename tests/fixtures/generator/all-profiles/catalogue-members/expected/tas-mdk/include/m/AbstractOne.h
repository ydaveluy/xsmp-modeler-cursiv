// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : AbstractOne.h
// -----------------------------------------------------------------------------
/// @file m/AbstractOne.h

#ifndef M_ABSTRACTONE_H_
#define M_ABSTRACTONE_H_

// Include the generated header file
#include <m/AbstractOneGen.h>

namespace m
{
/// A model.
class AbstractOne: public AbstractOneGen {
public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    AbstractOne(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~AbstractOne() noexcept override = default;

    private:
    ::Smp::Bool ping(::Smp::Int32 value) override;
};
} // namespace m

#endif // M_ABSTRACTONE_H_
