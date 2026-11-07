// -----------------------------------------------------------------------------
// File Name    : NullDefaults.h
// -----------------------------------------------------------------------------
/// @file n/NullDefaults.h

#ifndef N_NULLDEFAULTS_H_
#define N_NULLDEFAULTS_H_

// Include the generated header file
#include <n/NullDefaultsGen.h>

namespace n
{
class NullDefaults: public NullDefaultsGen {
public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    NullDefaults(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~NullDefaults() noexcept override = default;

    private:
    void ping(::Smp::Int32* a = nullptr, ::Smp::Int32 b = 2, ::Smp::Int32* c = nullptr, ::Smp::Char8 d = '\0') override;
};
} // namespace n

#endif // N_NULLDEFAULTS_H_
