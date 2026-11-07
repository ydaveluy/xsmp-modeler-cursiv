// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Payload.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Payload.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>
#include <top/inner/Payload.h>

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
constexpr ::Smp::Bool Payload::Second;

constexpr ::Smp::Bool Payload::First;

constexpr ::Smp::Int32 Payload::Answer;

const ::top::inner::Label Payload::Greeting{"hi \"there\""};

// Field shared
::Smp::Int32 Payload::shared{7};

void Payload::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddStructureType(
        "Payload", // Name
        "A payload.", // Description
        ::top::inner::Uuid_Payload // UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "enabled", // Name
        "A flag.", // Description
        ::Smp::Uuids::Uuid_Bool, // Type UUID
        offsetof(Payload, enabled), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "count", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(Payload, count), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        false, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "ratio", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Float64, // Type UUID
        offsetof(Payload, ratio), // Field offset
        ::Smp::ViewKind::VK_Debug, // View Kind
        true, // State
        true, // Input
        true // Output
    );
    type->AddField(
        "colour", // Name
        "", // Description
        ::top::inner::Uuid_Colour, // Type UUID
        offsetof(Payload, colour), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "values", // Name
        "", // Description
        ::top::inner::Uuid_Ints, // Type UUID
        offsetof(Payload, values), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "grid", // Name
        "", // Description
        ::top::inner::Uuid_Matrix, // Type UUID
        offsetof(Payload, grid), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "name", // Name
        "", // Description
        ::top::inner::Uuid_Label, // Type UUID
        offsetof(Payload, name), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "changing", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(Payload, changing), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "span", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Duration, // Type UUID
        offsetof(Payload, span), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "when", // Name
        "", // Description
        ::Smp::Uuids::Uuid_DateTime, // Type UUID
        offsetof(Payload, when), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "letter", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int8, // Type UUID
        offsetof(Payload, letter), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace inner
} // namespace top
