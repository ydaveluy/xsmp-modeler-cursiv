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
#include <TasMdk/Types/Structure.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace defaults
{
struct Packet
{
    ::defaults::Row samples;
    ::Smp::Int32 tag;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Packet.
constexpr ::Smp::Uuid Uuid_Packet { 0x70000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000007U };

template<bool ...Opts>
struct _Packet : public ::TasMdk::Types::StructureType<Opts...>
{
    // the equivalent raw_type
    using raw_type = ::defaults::Packet;

    // constructor
    _Packet(const std::string& name, const std::string& description,
            Smp::IObject *parent, Smp::ViewKind view,
            const Smp::Publication::ITypeRegistry *type_registry, Smp::Uuid typeUuid,
            const raw_type &default_value = raw_type{}) :
            ::TasMdk::Types::StructureType<Opts...>(name, description, parent, view, type_registry->GetType(typeUuid)),
            /// samples initialization
            samples{"samples", "", this, view, type_registry, ::defaults::Uuid_Row, default_value.samples},
            /// tag initialization
            tag{"tag", "", this, view, type_registry, ::Smp::Uuids::Uuid_Int32, default_value.tag} {
    }

    // copy operator
    _Packet & operator=(const _Packet &other)
    {
        samples = other.samples;
        tag = other.tag;
        return *this;
    }

    // copy operator from raw_type
    _Packet & operator=(const raw_type &other)
    {
        samples = other.samples;
        tag = other.tag;
        return *this;
    }

    // convert to raw_type
    operator raw_type() const noexcept
    {
        return {samples, tag};
    }
    typename ::TasMdk::ArrayField<::defaults::_Row>::in<Opts...>::type samples;
    typename ::TasMdk::SimpleField<::Smp::Int32>::in<Opts...>::type tag;
};
} // namespace defaults

#endif // DEFAULTS_PACKET_H_
