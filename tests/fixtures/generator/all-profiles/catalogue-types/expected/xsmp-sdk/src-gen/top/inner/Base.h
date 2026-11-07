// Copyright line one
// Copyright line two
// -----------------------------------------------------------------------------
// File Name    : Base.h
// -----------------------------------------------------------------------------
/// @file top/inner/Base.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef TOP_INNER_BASE_H_
#define TOP_INNER_BASE_H_

// ----------------------------------------------------------------------------
// --------------------------- Include Header Files ---------------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <top/inner/Payload.h>

// ----------------------------------------------------------------------------
// --------------------------- Types and Interfaces ---------------------------
// ----------------------------------------------------------------------------

/// Inner namespace doc.
///  Second line.
namespace top::inner
{
/// A base class.
class Base
{
    public:
    static void _Register(::Smp::Publication::ITypeRegistry* registry);

    Base() = default;
    ~Base() noexcept = default;
    Base(const Base&) = default;

    private:
    static constexpr ::Smp::Int32 K{1};
    public:
    /// Get p1.
    /// @return Current value of property p1.
    ::Smp::Int32 get_p1();
    /// Set p1.
    /// @param value New value of property p1 to set.
    void set_p1(::Smp::Int32 value);
    /// Get p2.
    /// @return Current value of property p2.
    virtual ::Smp::Int32 get_p2();
    /// Set p2.
    /// @param value New value of property p2 to set.
    virtual void set_p2(::Smp::Int32 value);
    /// Get p3.
    /// @return Current value of property p3.
    ::Smp::Int32 get_p3();
    /// Set p4.
    /// @param value New value of property p4 to set.
    void set_p4(::Smp::Int32 value);
    /// Get p5.
    /// @return Current value of property p5.
    static ::Smp::Int32 get_p5();
    /// Set p5.
    /// @param value New value of property p5 to set.
    static void set_p5(::Smp::Int32 value);
    void reset();
    ::Smp::Int32 count();
    virtual ::Smp::Int32 virt();
    ::Smp::Int32 viewed() const;
    static ::Smp::Int32 stat();
    ::Smp::Int32 a{1};
    protected:
    ::Smp::Int32 b{2};
    private:
    ::Smp::Int32 c{3};
    public:
    static ::Smp::Int32 d;
    mutable ::Smp::Int32 as1;
    static ::Smp::Int32 as2;
    ::top::inner::Payload as3;
};

/// Universally unique identifier of type Base.
inline constexpr ::Smp::Uuid Uuid_Base { 0x10000000U, 0x0000U, 0x4000U, 0x8000U, 0x000000000017U };
} // namespace top::inner

#endif // TOP_INNER_BASE_H_
