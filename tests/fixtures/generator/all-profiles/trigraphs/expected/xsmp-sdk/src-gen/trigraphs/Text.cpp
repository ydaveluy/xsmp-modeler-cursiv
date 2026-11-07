// -----------------------------------------------------------------------------
// File Name    : Text.cpp
// -----------------------------------------------------------------------------
/// @file trigraphs/Text.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <cstddef>
#include <Smp/Publication/IStructureType.h>
#include <Smp/ViewKind.h>
#include <trigraphs/Text.h>

namespace trigraphs
{
void Text::_Register(::Smp::Publication::ITypeRegistry* registry)
{
    registry->AddStructureType(
        "Text", // Name
        "Text ??= with all nine trigraphs.", // Description
        ::trigraphs::Uuid_Text // UUID
    );
}
} // namespace trigraphs
