// -----------------------------------------------------------------------------
// File Name    : WithParameters.h
// -----------------------------------------------------------------------------
/// @file n/WithParameters.h

#ifndef N_WITHPARAMETERS_H_
#define N_WITHPARAMETERS_H_

// Include the generated header file
#include <n/WithParametersGen.h>

namespace n
{
class WithParameters: public WithParametersGen {
public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    WithParameters(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~WithParameters() noexcept override = default;

    private:
    void ping(::Smp::Int32 count) override;
};
} // namespace n

#endif // N_WITHPARAMETERS_H_
