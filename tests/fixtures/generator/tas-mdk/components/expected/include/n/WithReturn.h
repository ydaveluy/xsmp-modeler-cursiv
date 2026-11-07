// -----------------------------------------------------------------------------
// File Name    : WithReturn.h
// -----------------------------------------------------------------------------
/// @file n/WithReturn.h

#ifndef N_WITHRETURN_H_
#define N_WITHRETURN_H_

// Include the generated header file
#include <n/WithReturnGen.h>

namespace n
{
class WithReturn: public WithReturnGen {
public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    WithReturn(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~WithReturn() noexcept override = default;

    private:
    ::Smp::Bool ready() override;
};
} // namespace n

#endif // N_WITHRETURN_H_
