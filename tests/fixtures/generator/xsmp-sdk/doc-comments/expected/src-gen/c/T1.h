// -----------------------------------------------------------------------------
// File Name    : T1.h
// -----------------------------------------------------------------------------
/// @file c/T1.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef C_T1_H_
#define C_T1_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace c
{
/// First paragraph with 'quotes' and "double" and \backslash.
/// Second paragraph.
///   Indented line.
/// @deprecated Do not use.
/// @since 1.0
/// @see
///other {@link Foo the foo} and more
struct T1 {
    /// One line.
    ::Smp::Int32 a;
    /// Line one
    /// Line two
    /// @custom
    ///tag text
    /// continued
    ::Smp::Int32 b;
    /// Described.
    ::Smp::Int32 c;
    ::Smp::Int32 d;
    /// Doc then line comment.
    ::Smp::Int32 e;
    /// Doc then plain block.
    ::Smp::Int32 f;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::c::T1{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::c::T1 &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view),
            // Field a
            a{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Int32, //Type UUID
                "a", // Name
                "One line.", // Description
                this, // Parent
                view, // View Kind
                value.a // Value
            },
            // Field b
            b{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Int32, //Type UUID
                "b", // Name
                "Line one\n Line two", // Description
                this, // Parent
                view, // View Kind
                value.b // Value
            },
            // Field c
            c{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Int32, //Type UUID
                "c", // Name
                "Described.", // Description
                this, // Parent
                view, // View Kind
                value.c // Value
            },
            // Field d
            d{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Int32, //Type UUID
                "d", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.d // Value
            },
            // Field e
            e{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Int32, //Type UUID
                "e", // Name
                "Doc then line comment.", // Description
                this, // Parent
                view, // View Kind
                value.e // Value
            },
            // Field f
            f{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Int32, //Type UUID
                "f", // Name
                "Doc then plain block.", // Description
                this, // Parent
                view, // View Kind
                value.f // Value
            }
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::c::T1
        _Field & operator=(const ::c::T1 &other) {
            this->a = other.a;
            this->b = other.b;
            this->c = other.c;
            this->d = other.d;
            this->e = other.e;
            this->f = other.f;
            return *this;
        }

        // implicit convertion to ::c::T1
        operator ::c::T1() const noexcept {
            return {a, b, c, d, e, f};
        }

        // Fields declaration
        /// One line.
        typename _BASE::template Field<::Smp::Int32> a;
        /// Line one
        /// Line two
        /// @custom
        ///tag text
        /// continued
        typename _BASE::template Field<::Smp::Int32> b;
        /// Described.
        typename _BASE::template Field<::Smp::Int32> c;
        typename _BASE::template Field<::Smp::Int32> d;
        /// Doc then line comment.
        typename _BASE::template Field<::Smp::Int32> e;
        /// Doc then plain block.
        typename _BASE::template Field<::Smp::Int32> f;
    };
};

/// Universally unique identifier of type T1.
inline constexpr ::Smp::Uuid Uuid_T1 { 0x30000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000001U };
} // namespace c

#endif // C_T1_H_
