// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Uses.h
// -----------------------------------------------------------------------------
/// @file top/other/Uses.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_OTHER_USES_H_
#define TOP_OTHER_USES_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>
#include <TasMdk/Types/Structure.h>
#include <top/inner/Colour.h>
#include <top/inner/Payload.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Top namespace doc.
namespace top
{
namespace other
{
struct Uses
{
    static constexpr ::Smp::Int32 V{::top::inner::Payload::Answer + 1};
    ::top::inner::Payload p;
    ::top::inner::Colour c{::top::inner::Colour::Red};

    Uses() = default;
    ~Uses() = default;
    Uses(const Uses&) = default;
    Uses(Uses&&) = default;
    Uses(::top::inner::Payload p, ::top::inner::Colour c):
    p(p), c(c) {}
    Uses& operator=(const Uses&) = default;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Uses.
constexpr ::Smp::Uuid Uuid_Uses { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000020U };

template<bool ...Opts>
struct _Uses : public ::TasMdk::Types::StructureType<Opts...>
{
    // the equivalent raw_type
    using raw_type = ::top::other::Uses;

    // constructor
    _Uses(const std::string& name, const std::string& description,
            Smp::IObject *parent, Smp::ViewKind view,
            const Smp::Publication::ITypeRegistry *type_registry, Smp::Uuid typeUuid,
            const raw_type &default_value = raw_type{}) :
            ::TasMdk::Types::StructureType<Opts...>(name, description, parent, view, type_registry->GetType(typeUuid)),
            /// p initialization
            p{"p", "", this, view, type_registry, ::top::inner::Uuid_Payload, default_value.p},
            /// c initialization
            c{"c", "", this, view, type_registry, ::top::inner::Uuid_Colour, default_value.c} {
    }

    // copy operator
    _Uses & operator=(const _Uses &other)
    {
        p = other.p;
        c = other.c;
        return *this;
    }

    // copy operator from raw_type
    _Uses & operator=(const raw_type &other)
    {
        p = other.p;
        c = other.c;
        return *this;
    }

    // convert to raw_type
    operator raw_type() const noexcept
    {
        return {p, c};
    }
    typename ::TasMdk::StructureField<::top::inner::_Payload>::in<Opts...>::type p;
    typename ::TasMdk::SimpleField<::top::inner::Colour>::in<Opts...>::type c;
};
} // namespace other
} // namespace top

#endif // TOP_OTHER_USES_H_
