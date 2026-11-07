// -----------------------------------------------------------------------------
// File Name    : Sensor.h
// -----------------------------------------------------------------------------
/// @file n/Sensor.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_SENSOR_H_
#define N_SENSOR_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
class Sensor
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    Sensor() = default;
    ~Sensor() noexcept = default;
    Sensor(const Sensor&) = default;

    private:
    /// Get Value.
    /// @return Current value of property Value.
    ::Smp::Float64 get_Value();
    /// Set Value.
    /// @param value New value of property Value to set.
    void set_Value(::Smp::Float64 value);
    ::Smp::Float64 raw;
};

/// Universally unique identifier of type Sensor.
inline constexpr ::Smp::Uuid Uuid_Sensor { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000045U };
} // namespace n

#endif // N_SENSOR_H_
