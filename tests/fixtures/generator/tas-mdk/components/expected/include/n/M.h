// -----------------------------------------------------------------------------
// File Name    : M.h
// -----------------------------------------------------------------------------
/// @file n/M.h

#ifndef N_M_H_
#define N_M_H_

// Include the generated header file
#include <n/MGen.h>

namespace n
{
class M: public MGen {
public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    M(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~M() noexcept override = default;

    private:
    void ope_reset() override;
    public:
    void _ept_step() override;
};
} // namespace n

#endif // N_M_H_
