// -----------------------------------------------------------------------------
// File Name    : WithReturn.cpp
// -----------------------------------------------------------------------------
/// @file n/WithReturn.cpp

#include <n/WithReturn.h>

namespace n
{
WithReturn::WithReturn(
        ::Smp::String8 name,
        ::Smp::String8 description,
        ::Smp::IObject* parent,
        ::Smp::Publication::ITypeRegistry* type_registry)
        : WithReturnGen::WithReturnGen(name, description, parent, type_registry) {
    // Publish_Callback = std::bind(&WithReturn::_publishHook, this, std::placeholders::_1);

    // Configure_Callback = std::bind(&WithReturn::_configureHook, this, std::placeholders::_1, std::placeholders::_2);

    // Connect_Callback = std::bind(&WithReturn::_connectHook, this, std::placeholders::_1);

    // Disconnect_Callback = std::bind(&WithReturn::_disconnectHook, this);
}

::Smp::Bool WithReturn::ready() {
    // TODO
    return false;
}
} // namespace n
