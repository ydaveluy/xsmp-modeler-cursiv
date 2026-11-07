// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Simple.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Simple.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/IArrayType.h>
#include <top/inner/Simple.h>

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
void _Register_Simple(::Smp::Publication::ITypeRegistry* registry) {
    registry->AddArrayType(
        "Simple", // Name
        "", // Description
        ::top::inner::Uuid_Simple, // UUID
        ::Smp::Uuids::Uuid_Bool, // Item Type UUID
        sizeof(::Smp::Bool), // Item Type size
        2, // Number of elements
        true // Simple array
    );
}
} // namespace inner
} // namespace top
