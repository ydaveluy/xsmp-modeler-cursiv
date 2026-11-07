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
#include <TasMdk/Types/Structure.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Top namespace doc.
namespace top
{
/// Inner namespace doc.
///  Second line.
namespace inner
{
struct Empty
{
    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Empty.
constexpr ::Smp::Uuid Uuid_Empty { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000016U };

template<bool ...Opts>
struct _Empty : public ::TasMdk::Types::StructureType<Opts...>
{
    // the equivalent raw_type
    using raw_type = ::top::inner::Empty;

    // constructor
    _Empty(const std::string& name, const std::string& description,
            Smp::IObject *parent, Smp::ViewKind view,
            const Smp::Publication::ITypeRegistry *type_registry, Smp::Uuid typeUuid,
            const raw_type &default_value = raw_type{}) :
            ::TasMdk::Types::StructureType<Opts...>(name, description, parent, view, type_registry->GetType(typeUuid)) {
    }

    // copy operator
    _Empty & operator=(const _Empty &other)
    {
        return *this;
    }

    // copy operator from raw_type
    _Empty & operator=(const raw_type &other)
    {
        return *this;
    }

    // convert to raw_type
    operator raw_type() const noexcept
    {
        return {};
    }
};
} // namespace inner
} // namespace top

#endif // TOP_INNER_EMPTY_H_
