// -----------------------------------------------------------------------------
// File Name    : Packet.h
// -----------------------------------------------------------------------------
/// @file defaults/Packet.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DEFAULTS_PACKET_H_
#define DEFAULTS_PACKET_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <defaults/Row.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace defaults
{
struct Packet {
    ::defaults::Row samples;
    ::Smp::Int32 tag;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    template<typename _BASE> struct _Field : public _BASE {
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
               ::Smp::Uuid typeUuid, ::Smp::String8 name,
               ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All)
            : _Field(typeRegistry, typeUuid, name, description, parent, view,
                     ::defaults::Packet{}) {}
        _Field(::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::Uuid typeUuid,
               ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
               ::Smp::ViewKind view, const ::defaults::Packet &value) :
            _BASE(typeRegistry, typeUuid, name, description, parent, view),
            // Field samples
            samples{
                typeRegistry, // Type Registry
                ::defaults::Uuid_Row, //Type UUID
                "samples", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.samples // Value
            },
            // Field tag
            tag{
                typeRegistry, // Type Registry
                ::Smp::Uuids::Uuid_Int32, //Type UUID
                "tag", // Name
                "", // Description
                this, // Parent
                view, // View Kind
                value.tag // Value
            }
        {
        }
        _Field(const _Field&) = delete;
        _Field& operator=(const _Field&) = delete;

        // copy operator from ::defaults::Packet
        _Field & operator=(const ::defaults::Packet &other) {
            this->samples = other.samples;
            this->tag = other.tag;
            return *this;
        }

        // implicit convertion to ::defaults::Packet
        operator ::defaults::Packet() const noexcept {
            return {samples, tag};
        }

        // Fields declaration
        typename _BASE::template Field<::defaults::Row> samples;
        typename _BASE::template Field<::Smp::Int32> tag;
    };
};

/// Universally unique identifier of type Packet.
inline constexpr ::Smp::Uuid Uuid_Packet { 0x70000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000007U };
} // namespace defaults

#endif // DEFAULTS_PACKET_H_
