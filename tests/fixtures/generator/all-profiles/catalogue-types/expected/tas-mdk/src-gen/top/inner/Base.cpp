// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Base.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Base.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <Smp/Publication/IClassType.h>
#include <Smp/Publication/IPublishOperation.h>
#include <top/inner/Base.h>

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
constexpr ::Smp::Int32 Base::K;

// Field d
::Smp::Int32 Base::d{4};

::Smp::Int32 Base::get_p1(){
    return this->a;
}
void Base::set_p1(::Smp::Int32 value) {
    this->a = value;
}

::Smp::Int32 Base::get_p5(){
    return Base::d;
}
void Base::set_p5(::Smp::Int32 value) {
    Base::d = value;
}

::Smp::Int32 Base::as2{};

void Base::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    auto *type = registry->AddClassType(
        "Base", // Name
        "A base class.", // Description
        ::top::inner::Uuid_Base, // UUID
        ::Smp::Uuids::Uuid_Void // Base Class UUID
    );

    // Register the Fields of the Class
    type->AddField(
        "a", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(Base, a), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "b", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(Base, b), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
    type->AddField(
        "c", // Name
        "", // Description
        ::Smp::Uuids::Uuid_Int32, // Type UUID
        offsetof(Base, c), // Field offset
        ::Smp::ViewKind::VK_All, // View Kind
        true, // State
        false, // Input
        false // Output
    );
}
} // namespace inner
} // namespace top
