// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Matrix.h
// -----------------------------------------------------------------------------
/// @file top/inner/Matrix.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_MATRIX_H_
#define TOP_INNER_MATRIX_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/ITypeRegistry.h>
#include <TasMdk/Types/Array.h>
#include <top/inner/Ints.h>

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
using Matrix = ::TasMdk::Types::Array<::top::inner::Ints, 2>;

/// Universally unique identifier of type Matrix.
constexpr ::Smp::Uuid Uuid_Matrix { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x00000000000eU };

void _Register_Matrix(::Smp::Publication::ITypeRegistry* registry);

template<bool ...Opts>
using _Matrix = TasMdk::Types::ArrayType<::top::inner::_Ints, 2, Opts...>;
} // namespace inner
} // namespace top

#endif // TOP_INNER_MATRIX_H_
