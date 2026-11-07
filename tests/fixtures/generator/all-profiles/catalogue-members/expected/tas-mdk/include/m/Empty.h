// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : Empty.h
// -----------------------------------------------------------------------------
/// @file m/Empty.h

#ifndef M_EMPTY_H_
#define M_EMPTY_H_

// Include the generated header file
#include <m/EmptyGen.h>

namespace m
{
class Empty: public EmptyGen {
public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    Empty(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~Empty() noexcept override = default;
};
} // namespace m

#endif // M_EMPTY_H_
