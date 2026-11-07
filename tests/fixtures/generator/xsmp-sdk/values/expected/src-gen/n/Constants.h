// -----------------------------------------------------------------------------
// File Name    : Constants.h
// -----------------------------------------------------------------------------
/// @file n/Constants.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_CONSTANTS_H_
#define N_CONSTANTS_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
struct Constants {
    static constexpr ::Smp::Float64 Pi{3.141592653589793};
    static constexpr ::Smp::Float64 Small{1.23456789012345e-20};

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::n::Constants{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::n::Constants &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view)
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::n::Constants
        _Field & operator=(const ::n::Constants &other) {
            return *this;
        }

        // implicit convertion to ::n::Constants
        operator ::n::Constants() const noexcept {
            return {};
        }
    };
};

/// Universally unique identifier of type Constants.
inline constexpr ::Smp::Uuid Uuid_Constants { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000092U };
} // namespace n

#endif // N_CONSTANTS_H_
