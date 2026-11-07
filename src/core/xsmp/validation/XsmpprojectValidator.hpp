#pragma once

#include <cursiv/services/CoreServices.hpp>
#include <cursiv/validation/ValidationAcceptor.hpp>
#include <cursiv/validation/ValidationRegistry.hpp>
#include <xsmp/ast.hpp>

namespace xsmp {

/// Project descriptor validation: standards, source roots, dependency cycles,
/// duplicate names and contribution aliases. Source existence uses the
/// configured filesystem provider, so disk and in-memory workspaces share the
/// same checks.
class XsmpprojectValidator final {
 public:
  explicit XsmpprojectValidator(const cursiv::CoreServices& services);

  void checkProject(const Project& project,
                    cursiv::ValidationAcceptor& accept) const;

  void checkProfile(const Profile& profile,
                    cursiv::ValidationAcceptor& accept) const;

  void checkTool(const Tool& tool, cursiv::ValidationAcceptor& accept) const;

 private:
  const cursiv::CoreServices& _services;
};

template <typename Services>
void registerXsmpprojectValidationChecks(Services& services) {
  using Registry = cursiv::ValidationRegistry;
  auto& registry = *services.validationRegistry;
  auto& validator = *services.validator;
  registry.registerChecks({
      Registry::makeValidationCheck<&XsmpprojectValidator::checkProject>(
          validator),
      Registry::makeValidationCheck<&XsmpprojectValidator::checkProfile>(
          validator),
      Registry::makeValidationCheck<&XsmpprojectValidator::checkTool>(
          validator),
  });
}

}  // namespace xsmp
