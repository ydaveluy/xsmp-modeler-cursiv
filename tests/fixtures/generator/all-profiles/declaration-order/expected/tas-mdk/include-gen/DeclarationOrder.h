// ---------------------------------------------------------------------------
// File Name    : DeclarationOrder.h
// ---------------------------------------------------------------------------
/// @file DeclarationOrder.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef DECLARATIONORDER_H_
#define DECLARATIONORDER_H_

// ----------------------------------------------------------------------------
// --------------------------------- Includes ---------------------------------
// ----------------------------------------------------------------------------
#include <Smp/ISimulator.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Version.h>

// Entry points for static library
extern "C" {
    /// Initialise Package DeclarationOrder.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_DeclarationOrder(
        ::Smp::ISimulator* simulator,
        ::Smp::Publication::ITypeRegistry* typeRegistry);

    /// Finalise Package DeclarationOrder.
    /// @return True if finalisation was successful, false otherwise.
    #if ECSS_SMP_VERSION < 202503L
    bool Finalise_DeclarationOrder();
    #else
    bool Finalise_DeclarationOrder(::Smp::ISimulator *simulator);
    #endif
}

#endif // DECLARATIONORDER_H_
