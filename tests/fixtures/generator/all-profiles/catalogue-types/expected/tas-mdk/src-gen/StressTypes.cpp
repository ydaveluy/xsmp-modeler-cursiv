// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : StressTypes.cpp
// -----------------------------------------------------------------------------
/// @file StressTypes.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <StressTypes.h>
#include <top/inner/Bare.h>
#include <top/inner/Base.h>
#include <top/inner/BigU.h>
#include <top/inner/Colour.h>
#include <top/inner/Derived.h>
#include <top/inner/Empty.h>
#include <top/inner/Initialised.h>
#include <top/inner/Ints.h>
#include <top/inner/Label.h>
#include <top/inner/Labels.h>
#include <top/inner/Matrix.h>
#include <top/inner/Nested.h>
#include <top/inner/NotSimple.h>
#include <top/inner/Oops.h>
#include <top/inner/Oops2.h>
#include <top/inner/Pair.h>
#include <top/inner/Payload.h>
#include <top/inner/Plain.h>
#include <top/inner/Pure.h>
#include <top/inner/R1.h>
#include <top/inner/R2.h>
#include <top/inner/R3.h>
#include <top/inner/R4.h>
#include <top/inner/Ratio.h>
#include <top/inner/Simple.h>
#include <top/inner/SmallU.h>
#include <top/inner/Speed.h>
#include <top/inner/WithDefaultCtor.h>
#include <top/other/Earlier.h>
#include <top/other/Kid.h>
#include <top/other/Later.h>
#include <top/other/Later2.h>
#include <top/other/Parent.h>
#include <top/other/Uses.h>
#include <unordered_set>

// -----------------------------------------------------------------------------
// ----------------------------- Global variables ------------------------------
// -----------------------------------------------------------------------------

namespace {
/// Simulators set.
std::unordered_set<::Smp::ISimulator*> simulators { };
} // namespace

// -----------------------------------------------------------------------------
// --------------------------- Initialise Function -----------------------------
// -----------------------------------------------------------------------------

extern "C"
{
    /// Initialise Package StressTypes.
    /// @param simulator Simulator for registration of factories.
    /// @param typeRegistry Type Registry for registration of types.
    /// @return True if initialisation was successful, false otherwise.
    bool Initialise_StressTypes(
            ::Smp::ISimulator* simulator,
            ::Smp::Publication::ITypeRegistry* typeRegistry) {
        // check simulator validity
        if (!simulator) {
            return false;
        }
        // avoid double initialisation
        if (!::simulators.emplace(simulator).second) {
            return true;
        }

        // register Enumeration Colour
        ::top::inner::_Register_Colour(typeRegistry);
        // register Integer Speed
        ::top::inner::_Register_Speed(typeRegistry);
        // register Integer Plain
        ::top::inner::_Register_Plain(typeRegistry);
        // register Integer BigU
        ::top::inner::_Register_BigU(typeRegistry);
        // register Integer SmallU
        ::top::inner::_Register_SmallU(typeRegistry);
        // register Float Ratio
        ::top::inner::_Register_Ratio(typeRegistry);
        // register Float R1
        ::top::inner::_Register_R1(typeRegistry);
        // register Float R2
        ::top::inner::_Register_R2(typeRegistry);
        // register Float R3
        ::top::inner::_Register_R3(typeRegistry);
        // register Float R4
        ::top::inner::_Register_R4(typeRegistry);
        // register String Label
        ::top::inner::_Register_Label(typeRegistry);
        // register Array Ints
        ::top::inner::_Register_Ints(typeRegistry);
        // register Array Simple
        ::top::inner::_Register_Simple(typeRegistry);
        // register Array NotSimple
        ::top::inner::_Register_NotSimple(typeRegistry);
        // register Structure Empty
        ::top::inner::Empty::_Register(typeRegistry);
        // register Class Base
        ::top::inner::Base::_Register(typeRegistry);
        // register Class Pure
        ::top::inner::Pure::_Register(typeRegistry);
        // register Structure Pair
        ::top::inner::Pair::_Register(typeRegistry);
        // register Class Bare
        ::top::inner::Bare::_Register(typeRegistry);
        // register Class WithDefaultCtor
        ::top::inner::WithDefaultCtor::_Register(typeRegistry);
        // register Exception Oops
        ::top::inner::Oops::_Register(typeRegistry);
        // register Array Later2
        ::top::other::_Register_Later2(typeRegistry);
        // register Class Parent
        ::top::other::Parent::_Register(typeRegistry);
        // register Array Labels
        ::top::inner::_Register_Labels(typeRegistry);
        // register Array Matrix
        ::top::inner::_Register_Matrix(typeRegistry);
        // register Class Derived
        ::top::inner::Derived::_Register(typeRegistry);
        // register Structure Initialised
        ::top::inner::Initialised::_Register(typeRegistry);
        // register Exception Oops2
        ::top::inner::Oops2::_Register(typeRegistry);
        // register Structure Earlier
        ::top::other::Earlier::_Register(typeRegistry);
        // register Class Kid
        ::top::other::Kid::_Register(typeRegistry);
        // register Structure Payload
        ::top::inner::Payload::_Register(typeRegistry);
        // register Array Later
        ::top::other::_Register_Later(typeRegistry);
        // register Structure Nested
        ::top::inner::Nested::_Register(typeRegistry);
        // register Structure Uses
        ::top::other::Uses::_Register(typeRegistry);

        return true;
    }
}

// ---------------------------------------------------------------------------------
// ---------------------------- Finalise Function ------------------------------
// ---------------------------------------------------------------------------------

extern "C"
{
    /// Finalise Package StressTypes.
    /// @return True if finalisation was successful, false otherwise.
    #if ECSS_SMP_VERSION < 202503L
    bool Finalise_StressTypes() {
        // avoid double finalisation
        if (::simulators.empty()) {
            return true;
        }
        ::simulators.clear();

        return true;
    }
    #else
    bool Finalise_StressTypes(::Smp::ISimulator *simulator) {
        // avoid double finalisation
        if (::simulators.empty()) {
            return true;
        }
        // check simulator validity
        if (!simulator) {
            return false;
        }
        // avoid double finalisation
        if (!::simulators.erase(simulator)) {
            return true;
        }
        return true;
    }
    #endif
}
