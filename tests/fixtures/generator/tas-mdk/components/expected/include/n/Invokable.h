// -----------------------------------------------------------------------------
// File Name    : Invokable.h
// -----------------------------------------------------------------------------
/// @file n/Invokable.h

#ifndef N_INVOKABLE_H_
#define N_INVOKABLE_H_

// Include the generated header file
#include <n/InvokableGen.h>

namespace n
{
class Invokable: public InvokableGen {
public:
    using InvokableGen::InvokableGen;
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    Invokable(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~Invokable() noexcept override = default;

    private:
    ::Smp::Int32* get_pointer() override;
    void set_pointer(::Smp::Int32* value) override;
    ::Smp::Int32 get_plain() override;
    void set_plain(::Smp::Int32 value) override;
    void ping(::Smp::Int32 count) override;
};
} // namespace n

#endif // N_INVOKABLE_H_
