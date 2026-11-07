// -----------------------------------------------------------------------------
// File Name    : Sensor.cpp
// -----------------------------------------------------------------------------
/// @file n/Sensor.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/Sensor.h>
#include <Smp/Publication/IClassType.h>

namespace n
{
::Smp::Float64 Sensor::get_Value(){
    return this->raw;
}
void Sensor::set_Value(::Smp::Float64 value) {
    this->raw = value;
}

void Sensor::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddClassType(
        "Sensor", // Name
        "", // Description
        ::n::Uuid_Sensor, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "raw", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Float64, // Type UUID
        offsetof(Sensor, raw), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace n
