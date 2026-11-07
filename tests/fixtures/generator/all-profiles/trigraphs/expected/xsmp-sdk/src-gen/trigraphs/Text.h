// -----------------------------------------------------------------------------
// File Name    : Text.h
// -----------------------------------------------------------------------------
/// @file trigraphs/Text.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TRIGRAPHS_TEXT_H_
#define TRIGRAPHS_TEXT_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace trigraphs
{
/// Text ??= with all nine trigraphs.
struct Text {
    static constexpr ::Smp::String8 all{"??= ??/ ??' ??( ??) ??! ??< ??> ??-"};
    /// Ends with a backslash trigraph ??/
    static constexpr ::Smp::String8 normal{"backslash \\ and quoted \"question?\""};

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::trigraphs::Text{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::trigraphs::Text &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view)
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::trigraphs::Text
        _Field & operator=(const ::trigraphs::Text &other) {
            return *this;
        }

        // implicit convertion to ::trigraphs::Text
        operator ::trigraphs::Text() const noexcept {
            return {};
        }
    };
};

/// Universally unique identifier of type Text.
inline constexpr ::Smp::Uuid Uuid_Text { 0x82000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000002U };
} // namespace trigraphs

#endif // TRIGRAPHS_TEXT_H_
