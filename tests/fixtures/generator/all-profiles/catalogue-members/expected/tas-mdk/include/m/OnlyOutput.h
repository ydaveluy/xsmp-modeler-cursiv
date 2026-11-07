// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : OnlyOutput.h
// -----------------------------------------------------------------------------
/// @file m/OnlyOutput.h

#ifndef M_ONLYOUTPUT_H_
#define M_ONLYOUTPUT_H_

// Include the generated header file
#include <m/OnlyOutputGen.h>

namespace m
{
class OnlyOutput: public OnlyOutputGen {
public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    OnlyOutput(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~OnlyOutput() noexcept override = default;
};
} // namespace m

#endif // M_ONLYOUTPUT_H_
