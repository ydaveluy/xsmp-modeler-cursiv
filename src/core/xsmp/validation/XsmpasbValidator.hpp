#pragma once

#include <xsmp/validation/DeploymentValidator.hpp>

namespace xsmp {

// Checks of the assembly language (`.xsmpasb`), on top of the shared ones.
class XsmpasbValidator final : public DeploymentValidator {
 public:
  void checkAssembly(const Assembly& node,
                     cursiv::ValidationAcceptor& accept) const;

  void checkEventLink(const EventLink& node,
                      cursiv::ValidationAcceptor& accept) const;

  void checkFieldLink(const FieldLink& node,
                      cursiv::ValidationAcceptor& accept) const;

  void checkInterfaceLink(const InterfaceLink& node,
                          cursiv::ValidationAcceptor& accept) const;

  // Its name and implementation and, for its children, unique names and the
  // upper bound of each Container.
  void checkModelInstance(const ModelInstance& node,
                          cursiv::ValidationAcceptor& accept) const;

  // Its name, and that it does not instantiate the assembly it sits in, however
  // deep.
  void checkAssemblyInstance(const AssemblyInstance& node,
                             cursiv::ValidationAcceptor& accept) const;

  void checkSubInstance(const SubInstance& node,
                        cursiv::ValidationAcceptor& accept) const;

  void checkGlobalEventHandler(const GlobalEventHandler& node,
                               cursiv::ValidationAcceptor& accept) const;

  void checkOperationCall(const OperationCall& node,
                          cursiv::ValidationAcceptor& accept) const;

  void checkPropertyValue(const PropertyValue& node,
                          cursiv::ValidationAcceptor& accept) const;
};

template <typename Services>
void registerXsmpasbValidationChecks(Services& services) {
  using Registry = cursiv::ValidationRegistry;
  auto& registry = *services.validationRegistry;
  auto& validator = *services.validator;
  registry.registerChecks({
      Registry::makeValidationCheck<&XsmpasbValidator::checkPath>(validator),
      Registry::makeValidationCheck<&XsmpasbValidator::checkFieldValue>(
          validator),
      Registry::makeValidationCheck<&XsmpasbValidator::checkIntValue>(
          validator),
      Registry::makeValidationCheck<&XsmpasbValidator::checkFloatValue>(
          validator),
      Registry::makeValidationCheck<&XsmpasbValidator::checkArrayValue>(
          validator),
      Registry::makeValidationCheck<&XsmpasbValidator::checkChar8Value>(
          validator),
      Registry::makeValidationCheck<&XsmpasbValidator::checkInt32Parameter>(
          validator),
      Registry::makeValidationCheck<&XsmpasbValidator::checkStringParameter>(
          validator),
      Registry::makeValidationCheck<&XsmpasbValidator::checkAssembly>(
          validator),
      Registry::makeValidationCheck<&XsmpasbValidator::checkEventLink>(
          validator),
      Registry::makeValidationCheck<&XsmpasbValidator::checkFieldLink>(
          validator),
      Registry::makeValidationCheck<&XsmpasbValidator::checkInterfaceLink>(
          validator),
      Registry::makeValidationCheck<&XsmpasbValidator::checkModelInstance>(
          validator),
      Registry::makeValidationCheck<&XsmpasbValidator::checkAssemblyInstance>(
          validator),
      Registry::makeValidationCheck<&XsmpasbValidator::checkSubInstance>(
          validator),
      Registry::makeValidationCheck<&XsmpasbValidator::checkGlobalEventHandler>(
          validator),
      Registry::makeValidationCheck<&XsmpasbValidator::checkOperationCall>(
          validator),
      Registry::makeValidationCheck<&XsmpasbValidator::checkPropertyValue>(
          validator),
  });
}

}  // namespace xsmp
