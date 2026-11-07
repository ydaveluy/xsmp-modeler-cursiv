#pragma once

#include <xsmp/validation/DeploymentValidator.hpp>

namespace xsmp {

// Checks of the configuration language (`.xsmpcfg`), on top of the shared ones.
class XsmpcfgValidator final : public DeploymentValidator {
 public:
  void checkConfiguration(const Configuration& node,
                          cursiv::ValidationAcceptor& accept) const;

  void checkComponentConfiguration(const ComponentConfiguration& node,
                                   cursiv::ValidationAcceptor& accept) const;

  void checkConfigurationUsage(const ConfigurationUsage& node,
                               cursiv::ValidationAcceptor& accept) const;
};

template <typename Services>
void registerXsmpcfgValidationChecks(Services& services) {
  using Registry = cursiv::ValidationRegistry;
  auto& registry = *services.validationRegistry;
  auto& validator = *services.validator;
  registry.registerChecks({
      Registry::makeValidationCheck<&XsmpcfgValidator::checkPath>(validator),
      Registry::makeValidationCheck<&XsmpcfgValidator::checkFieldValue>(
          validator),
      Registry::makeValidationCheck<&XsmpcfgValidator::checkIntValue>(
          validator),
      Registry::makeValidationCheck<&XsmpcfgValidator::checkFloatValue>(
          validator),
      Registry::makeValidationCheck<&XsmpcfgValidator::checkArrayValue>(
          validator),
      Registry::makeValidationCheck<&XsmpcfgValidator::checkChar8Value>(
          validator),
      Registry::makeValidationCheck<&XsmpcfgValidator::checkConfiguration>(
          validator),
      Registry::makeValidationCheck<
          &XsmpcfgValidator::checkComponentConfiguration>(validator),
      Registry::makeValidationCheck<&XsmpcfgValidator::checkConfigurationUsage>(
          validator),
  });
}

}  // namespace xsmp
