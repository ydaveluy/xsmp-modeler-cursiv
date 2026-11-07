// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Earlier.h
// -----------------------------------------------------------------------------
/// @file top/other/Earlier.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_OTHER_EARLIER_H_
#define TOP_OTHER_EARLIER_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>
#include <TasMdk/Types/Structure.h>
#include <top/other/Later2.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Top namespace doc.
namespace top
{
namespace other
{
struct Earlier
{
    ::top::other::Later2 x;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Earlier.
constexpr ::Smp::Uuid Uuid_Earlier { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000022U };

template<bool ...Opts>
struct _Earlier : public ::TasMdk::Types::StructureType<Opts...>
{
    // the equivalent raw_type
    using raw_type = ::top::other::Earlier;

    // constructor
    _Earlier(const std::string& name, const std::string& description,
            Smp::IObject *parent, Smp::ViewKind view,
            const Smp::Publication::ITypeRegistry *type_registry, Smp::Uuid typeUuid,
            const raw_type &default_value = raw_type{}) :
            ::TasMdk::Types::StructureType<Opts...>(name, description, parent, view, type_registry->GetType(typeUuid)),
            /// x initialization
            x{"x", "", this, view, type_registry, ::top::other::Uuid_Later2, default_value.x} {
    }

    // copy operator
    _Earlier & operator=(const _Earlier &other)
    {
        x = other.x;
        return *this;
    }

    // copy operator from raw_type
    _Earlier & operator=(const raw_type &other)
    {
        x = other.x;
        return *this;
    }

    // convert to raw_type
    operator raw_type() const noexcept
    {
        return {x};
    }
    typename ::TasMdk::ArrayField<::top::other::_Later2>::in<Opts...>::type x;
};
} // namespace other
} // namespace top

#endif // TOP_OTHER_EARLIER_H_
