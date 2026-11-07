// -----------------------------------------------------------------------------
// File Name    : Skipped.h
// -----------------------------------------------------------------------------
/// @file n/Skipped.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_SKIPPED_H_
#define N_SKIPPED_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
struct Skipped {
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::n::Skipped{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::n::Skipped &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view)
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::n::Skipped
        _Field & operator=(const ::n::Skipped &other) {
            return *this;
        }

        // implicit convertion to ::n::Skipped
        operator ::n::Skipped() const noexcept {
            return {};
        }
    };
};

/// Universally unique identifier of type Skipped.
inline constexpr ::Smp::Uuid Uuid_Skipped { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000076U };
} // namespace n

#endif // N_SKIPPED_H_
