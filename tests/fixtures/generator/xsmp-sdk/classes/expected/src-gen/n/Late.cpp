// -----------------------------------------------------------------------------
// File Name    : Late.cpp
// -----------------------------------------------------------------------------
/// @file n/Late.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <n/Late.h>
#include <Smp/Publication/IClassType.h>

namespace n
{
void Late::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddClassType(
        "Late", // Name
        "", // Description
        ::n::Uuid_Late, // UUID
        ::n::Uuid_Early // Base Class UUID
    );
}
} // namespace n
