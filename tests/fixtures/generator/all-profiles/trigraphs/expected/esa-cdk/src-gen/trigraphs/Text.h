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
} // namespace trigraphs

#endif // TRIGRAPHS_TEXT_H_
