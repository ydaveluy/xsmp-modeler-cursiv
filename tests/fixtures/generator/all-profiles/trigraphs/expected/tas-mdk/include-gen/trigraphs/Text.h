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
#include <TasMdk/Types/Structure.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

namespace trigraphs
{
/// Text ?\?= with all nine trigraphs.
struct Text
{
    static constexpr ::Smp::String8 all{"?\?= ?\?/ ?\?' ?\?( ?\?) ?\?! ?\?< ?\?> ?\?-"};
    /// Ends with a backslash trigraph ?\?/
    static constexpr ::Smp::String8 normal{"backslash \\ and quoted \"question?\""};

    static void _Register(::Smp::Publication::ITypeRegistry* registry);
};

/// Universally unique identifier of type Text.
constexpr ::Smp::Uuid Uuid_Text { 0x82000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000002U };

template<bool ...Opts>
struct _Text : public ::TasMdk::Types::StructureType<Opts...>
{
    // the equivalent raw_type
    using raw_type = ::trigraphs::Text;

    // constructor
    _Text(const std::string& name, const std::string& description,
            Smp::IObject *parent, Smp::ViewKind view,
            const Smp::Publication::ITypeRegistry *type_registry, Smp::Uuid typeUuid,
            const raw_type &default_value = raw_type{}) :
            ::TasMdk::Types::StructureType<Opts...>(name, description, parent, view, type_registry->GetType(typeUuid)) {
    }

    // copy operator
    _Text & operator=(const _Text &other)
    {
        return *this;
    }

    // copy operator from raw_type
    _Text & operator=(const raw_type &other)
    {
        return *this;
    }

    // convert to raw_type
    operator raw_type() const noexcept
    {
        return {};
    }
};
} // namespace trigraphs

#endif // TRIGRAPHS_TEXT_H_
