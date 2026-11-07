// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Empty.h
// -----------------------------------------------------------------------------
/// @file top/inner/Empty.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_EMPTY_H_
#define TOP_INNER_EMPTY_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
struct Empty {
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::top::inner::Empty{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::top::inner::Empty &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view)
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::top::inner::Empty
        _Field & operator=(const ::top::inner::Empty &other) {
            return *this;
        }

        // implicit convertion to ::top::inner::Empty
        operator ::top::inner::Empty() const noexcept {
            return {};
        }
    };
};

/// Universally unique identifier of type Empty.
inline constexpr ::Smp::Uuid Uuid_Empty { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000016U };
} // namespace top::inner

#endif // TOP_INNER_EMPTY_H_
