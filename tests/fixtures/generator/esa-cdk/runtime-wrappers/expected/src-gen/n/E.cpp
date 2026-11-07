// -----------------------------------------------------------------------------
// File Name    : E.cpp
// -----------------------------------------------------------------------------
/// @file n/E.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/E.h>
#include <Smp/Publication/IEnumerationType.h>

namespace n
{
void _Register_E(::Smp::Publication::ITypeRegistry* registry) {
    auto *type = registry->AddEnumerationType(
        "E", // Name
        "", // Description
        ::n::Uuid_E, // UUID
        sizeof(::n::E) // Size
    );

    // Register the Literals of the Enumeration
    type->AddLiteral("A", "", static_cast<::Smp::Int32>(::n::E::A));
}
} // namespace n
