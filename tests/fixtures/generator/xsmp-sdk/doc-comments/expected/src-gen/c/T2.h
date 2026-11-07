// -----------------------------------------------------------------------------
// File Name    : T2.h
// -----------------------------------------------------------------------------
/// @file c/T2.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef C_T2_H_
#define C_T2_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace c
{
/// Doc then line comment on a type.
struct T2 {
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::c::T2{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::c::T2 &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view)
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::c::T2
        _Field & operator=(const ::c::T2 &other) {
            return *this;
        }

        // implicit convertion to ::c::T2
        operator ::c::T2() const noexcept {
            return {};
        }
    };
};

/// Universally unique identifier of type T2.
inline constexpr ::Smp::Uuid Uuid_T2 { 0x30000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000002U };
} // namespace c

#endif // C_T2_H_
