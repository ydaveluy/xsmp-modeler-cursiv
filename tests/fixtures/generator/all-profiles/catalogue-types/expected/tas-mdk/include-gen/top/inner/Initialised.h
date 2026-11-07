// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Initialised.h
// -----------------------------------------------------------------------------
/// @file top/inner/Initialised.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_INITIALISED_H_
#define TOP_INNER_INITIALISED_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/ViewKind.h>
#include <TasMdk/Types/Structure.h>
#include <top/inner/Pair.h>

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
struct Initialised
{
    ::top::inner::Pair byName{ /* .x = */1, /* .y = */2};
    ::top::inner::Pair byPosition{ 3, 4};

    Initialised() = default;
    ~Initialised() = default;
    Initialised(const Initialised&) = default;
    Initialised(Initialised&&) = default;
    Initialised(::top::inner::Pair byName, ::top::inner::Pair byPosition):
    byName(byName), byPosition(byPosition) {}
    Initialised& operator=(const Initialised&) = default;

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Initialised.
constexpr ::Smp::Uuid Uuid_Initialised { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000028U };

template<bool ...Opts>
struct _Initialised : public ::TasMdk::Types::StructureType<Opts...>
{
    // the equivalent raw_type
    using raw_type = ::top::inner::Initialised;

    // constructor
    _Initialised(const std::string& name, const std::string& description,
            Smp::IObject *parent, Smp::ViewKind view,
            const Smp::Publication::ITypeRegistry *type_registry, Smp::Uuid typeUuid,
            const raw_type &default_value = raw_type{}) :
            ::TasMdk::Types::StructureType<Opts...>(name, description, parent, view, type_registry->GetType(typeUuid)),
            /// byName initialization
            byName{"byName", "", this, view, type_registry, ::top::inner::Uuid_Pair, default_value.byName},
            /// byPosition initialization
            byPosition{"byPosition", "", this, view, type_registry, ::top::inner::Uuid_Pair, default_value.byPosition} {
    }

    // copy operator
    _Initialised & operator=(const _Initialised &other)
    {
        byName = other.byName;
        byPosition = other.byPosition;
        return *this;
    }

    // copy operator from raw_type
    _Initialised & operator=(const raw_type &other)
    {
        byName = other.byName;
        byPosition = other.byPosition;
        return *this;
    }

    // convert to raw_type
    operator raw_type() const noexcept
    {
        return {byName, byPosition};
    }
    typename ::TasMdk::StructureField<::top::inner::_Pair>::in<Opts...>::type byName;
    typename ::TasMdk::StructureField<::top::inner::_Pair>::in<Opts...>::type byPosition;
};
} // namespace inner
} // namespace top

#endif // TOP_INNER_INITIALISED_H_
