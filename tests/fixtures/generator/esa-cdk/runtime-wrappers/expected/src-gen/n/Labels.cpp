// -----------------------------------------------------------------------------
// File Name    : Labels.cpp
// -----------------------------------------------------------------------------
/// @file n/Labels.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Labels.h>
#include <Smp/Publication/IArrayType.h>

namespace n
{
void _Register_Labels(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Labels", // Name
        "", // Description
        ::n::Uuid_Labels, // UUID
        ::n::Uuid_Label, // Item Type UUID
        sizeof(::n::Label), // Item Type size
        2, // Number of elements
        false // Simple array
    );
}
} // namespace n
