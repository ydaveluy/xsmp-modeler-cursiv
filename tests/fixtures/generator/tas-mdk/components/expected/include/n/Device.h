// -----------------------------------------------------------------------------
// File Name    : Device.h
// -----------------------------------------------------------------------------
/// @file n/Device.h

#ifndef N_DEVICE_H_
#define N_DEVICE_H_

// Include the generated header file
#include <n/DeviceGen.h>

namespace n
{
class Device: public DeviceGen {
public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    Device(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~Device() noexcept override = default;

    private:
    ::Smp::Bool get_ready() override;
    void set_ready(::Smp::Bool value) override;
    void apply(::Smp::Int32 count = 42) override;
};
} // namespace n

#endif // N_DEVICE_H_
