// -----------------------------------------------------------------------------
// File Name    : Probe.h
// -----------------------------------------------------------------------------
/// @file limits/Probe.h

#ifndef LIMITS_PROBE_H_
#define LIMITS_PROBE_H_

// Include the generated header file
#include <limits/ProbeGen.h>

namespace limits
{
class Probe: public ProbeGen {
public:
    using ProbeGen::ProbeGen;
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
    ::Smp::Int32* pointer() override;
    ::Smp::Int32 calculate(::Smp::Int32 value) override;
    ::Smp::Int32 calculate(::Smp::Float64 value) override;
    ::Smp::Int32 normal(::Smp::Int32 value) override;
};
} // namespace limits

#endif // LIMITS_PROBE_H_
