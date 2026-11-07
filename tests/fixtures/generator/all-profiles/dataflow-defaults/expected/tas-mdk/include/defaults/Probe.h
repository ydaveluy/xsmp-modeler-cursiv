// -----------------------------------------------------------------------------
// File Name    : Probe.h
// -----------------------------------------------------------------------------
/// @file defaults/Probe.h

#ifndef DEFAULTS_PROBE_H_
#define DEFAULTS_PROBE_H_

// Include the generated header file
#include <defaults/ProbeGen.h>

namespace defaults
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
};
} // namespace defaults

#endif // DEFAULTS_PROBE_H_
