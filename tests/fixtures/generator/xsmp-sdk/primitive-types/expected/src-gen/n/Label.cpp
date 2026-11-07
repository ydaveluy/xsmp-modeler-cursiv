// -----------------------------------------------------------------------------
// File Name    : Label.cpp
// -----------------------------------------------------------------------------
/// @file n/Label.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Label.h>
#include <Smp/Publication/IType.h>

namespace n
{
void _Register_Label(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddStringType(
        "Label", // Name
        "A short label.", // Description
        ::n::Uuid_Label, // UUID
        16 // Length
    );
}
} // namespace n
