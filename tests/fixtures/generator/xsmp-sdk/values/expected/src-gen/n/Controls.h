// -----------------------------------------------------------------------------
// File Name    : Controls.h
// -----------------------------------------------------------------------------
/// @file n/Controls.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef N_CONTROLS_H_
#define N_CONTROLS_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <n/Name.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace n
{
struct Controls {
    static constexpr ::Smp::Char8 nul{'\0'};
    static constexpr ::Smp::Char8 control{'\037'};
    static constexpr ::Smp::String8 inside{"a\0b"};
    static constexpr ::Smp::String8 digit{"\0011"};
    static constexpr ::Smp::String8 nulDigit{"\0001"};
    static constexpr ::Smp::String8 erase{"\177"};
    static constexpr ::Smp::String8 path{"C:\\dir\n"};
    ::Smp::Char8 field1{'\002'};
    ::n::Name field2{"\0\003"};

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::n::Controls{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::n::Controls &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view),
            // Field field1
            field1{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Char8, //Type UUID
                "field1", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.field1 // Value
            },
            // Field field2
            field2{
                typeRegistry, // Type Registry
                ::n::Uuid_Name, //Type UUID
                "field2", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.field2 // Value
            }
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::n::Controls
        _Field & operator=(const ::n::Controls &other) {
            this->field1 = other.field1;
            this->field2 = other.field2;
            return *this;
        }

        // implicit convertion to ::n::Controls
        operator ::n::Controls() const noexcept {
            return {field1, field2};
        }

        // Fields declaration
        typename _BASE::template Field<::Smp::Char8> field1;
        typename _BASE::template Field<::n::Name> field2;
    };
};

/// Universally unique identifier of type Controls.
inline constexpr ::Smp::Uuid Uuid_Controls { 0x11111111U, 0x0000U, 0x4000U, 0x8000U, 0x000000000095U };
} // namespace n

#endif // N_CONTROLS_H_
