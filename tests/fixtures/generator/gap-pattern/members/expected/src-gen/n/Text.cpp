// -----------------------------------------------------------------------------
// File Name    : Text.cpp
// -----------------------------------------------------------------------------
/// @file n/Text.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Text.h>
#include <Smp/Publication/IType.h>

namespace n
{
void _Register_Text(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddStringType(
        "Text", // Name
        "", // Description
        ::n::Uuid_Text, // UUID
        4 // Length
    );
}
} // namespace n
