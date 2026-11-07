// A field of `other` and an operation of `types` chain their packages. An
// interface has no implementation, so that `interfaces` is not chained.
// -----------------------------------------------------------------------------
// File Name    : S.h
// -----------------------------------------------------------------------------
/// @file n2/S.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N2_S_H_
#define N2_S_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/P.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n2
{
struct S {
    ::n::P x;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::n2::S{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::n2::S &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view),
            // Field x
            x{
                typeRegistry, // Type Registry
                ::n::Uuid_P, //Type UUID
                "x", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.x // Value
            }
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::n2::S
        _Field & operator=(const ::n2::S &other) {
            this->x = other.x;
            return *this;
        }

        // implicit convertion to ::n2::S
        operator ::n2::S() const noexcept {
            return {x};
        }

        // Fields declaration
        typename _BASE::template Field<::n::P> x;
    };
};

/// Universally unique identifier of type S.
inline constexpr ::Smp::Uuid Uuid_S { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000004U };
} // namespace n2

#endif // N2_S_H_
