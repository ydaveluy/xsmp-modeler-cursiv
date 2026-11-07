// -----------------------------------------------------------------------------
// File Name    : T1.cpp
// -----------------------------------------------------------------------------
/// @file c/T1.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <c/T1.h>
#include <cstddef>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>

namespace c
{
void T1::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddStructureType(
        "T1", // Name
        "First paragraph with \'quotes\' and \"double\" and \\backslash.\n\n Second paragraph.\n   Indented line.", // Description
        ::c::Uuid_T1 // UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "a", // Name
        "One line.", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(T1, a), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "b", // Name
        "Line one\n Line two", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(T1, b), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "c", // Name
        "Described.", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(T1, c), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "d", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(T1, d), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "e", // Name
        "Doc then line comment.", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(T1, e), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "f", // Name
        "Doc then plain block.", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(T1, f), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace c
