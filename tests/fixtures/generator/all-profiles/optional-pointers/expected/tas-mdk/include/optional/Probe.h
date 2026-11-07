// -----------------------------------------------------------------------------
// File Name    : Probe.h
// -----------------------------------------------------------------------------
/// @file optional/Probe.h

#ifndef OPTIONAL_PROBE_H_
#define OPTIONAL_PROBE_H_

// Include the generated header file
#include <optional/ProbeGen.h>

namespace optional
{
class Probe: public ProbeGen {
public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    Probe(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~Probe() noexcept override = default;

    private:
    /// Optional input.
    void ope_input(::Smp::Int32* value = nullptr) override;
    /// The request owns the String8 until the handler returns.
    void ope_string(::Smp::String8* value = nullptr) override;
    /// Optional output.
    void ope_output(::Smp::Int32* value = nullptr) override;
    /// Optional input/output.
    void ope_inout(::Smp::Int32* value = nullptr) override;
    /// The parameter names must not collide with generated presence flags.
    void ope_names(::Smp::Int32* value = nullptr, ::Smp::Int32* value_present = nullptr) override;
};
} // namespace optional

#endif // OPTIONAL_PROBE_H_
