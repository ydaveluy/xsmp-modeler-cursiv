// Block notice
// line two
// -----------------------------------------------------------------------------
// File Name    : ServKid.h
// -----------------------------------------------------------------------------
/// @file m/ServKid.h

#ifndef M_SERVKID_H_
#define M_SERVKID_H_

// Include the generated header file
#include <m/ServKidGen.h>

namespace m
{
class ServKid: public ServKidGen {
public:
    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    /// @param type_registry Reference to global type registry.
    ServKid(
            ::Smp::String8 name,
            ::Smp::String8 description,
            ::Smp::IObject* parent,
            ::Smp::Publication::ITypeRegistry* type_registry);

    /// Virtual destructor to release memory.
    ~ServKid() noexcept override = default;
};
} // namespace m

#endif // M_SERVKID_H_
