#pragma once

#include <xsmp/validation/DeploymentValidator.hpp>

namespace xsmp {

// Checks of the link base language (`.xsmplnk`), on top of the shared ones.
class XsmplnkValidator final : public DeploymentValidator {
 public:
  void checkLinkBase(const LinkBase& node,
                     cursiv::ValidationAcceptor& accept) const;

  void checkComponentLinkBase(const ComponentLinkBase& node,
                              cursiv::ValidationAcceptor& accept) const;

  void checkInterfaceLink(const InterfaceLink& node,
                          cursiv::ValidationAcceptor& accept) const;

  void checkEventLink(const EventLink& node,
                      cursiv::ValidationAcceptor& accept) const;

  void checkFieldLink(const FieldLink& node,
                      cursiv::ValidationAcceptor& accept) const;
};

template <typename Services>
void registerXsmplnkValidationChecks(Services& services) {
  using Registry = cursiv::ValidationRegistry;
  auto& registry = *services.validationRegistry;
  auto& validator = *services.validator;
  registry.registerChecks({
      Registry::makeValidationCheck<&XsmplnkValidator::checkPath>(validator),
      Registry::makeValidationCheck<&XsmplnkValidator::checkLinkBase>(
          validator),
      Registry::makeValidationCheck<&XsmplnkValidator::checkComponentLinkBase>(
          validator),
      Registry::makeValidationCheck<&XsmplnkValidator::checkInterfaceLink>(
          validator),
      Registry::makeValidationCheck<&XsmplnkValidator::checkEventLink>(
          validator),
      Registry::makeValidationCheck<&XsmplnkValidator::checkFieldLink>(
          validator),
  });
}

}  // namespace xsmp
