// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Derived.cpp
// -----------------------------------------------------------------------------
/// @file top/inner/Derived.cpp

#include <top/inner/Derived.h>

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
void Derived::operator+=(::Smp::Int32 a) {
    // TODO
}

::Smp::Bool Derived::operator==(::top::inner::Derived other) {
    // TODO
    return false;
}
} // namespace top::inner
