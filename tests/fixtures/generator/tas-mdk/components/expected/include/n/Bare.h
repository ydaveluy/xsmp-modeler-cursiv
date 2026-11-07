// -----------------------------------------------------------------------------
// File Name    : Bare.h
// -----------------------------------------------------------------------------
/// @file n/Bare.h

#ifndef N_BARE_H_
#define N_BARE_H_

// Include the generated header file
#include <n/BareGen.h>

namespace n
{
class Bare: public BareGen {
public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    Bare(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~Bare() noexcept override = default;

    private:
    void reset() override;
};
} // namespace n

#endif // N_BARE_H_
