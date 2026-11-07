#pragma once

#include <xsmp/validation/DeploymentValidator.hpp>

namespace xsmp {

// Checks of the schedule language (`.xsmpsed`), on top of the shared ones.
class XsmpsedValidator final : public DeploymentValidator {
 public:
  void checkSchedule(const Schedule& node,
                     cursiv::ValidationAcceptor& accept) const;

  void checkTask(const Task& node, cursiv::ValidationAcceptor& accept) const;

  void checkMissionEvent(const MissionEvent& node,
                         cursiv::ValidationAcceptor& accept) const;

  void checkSimulationEvent(const SimulationEvent& node,
                            cursiv::ValidationAcceptor& accept) const;

  void checkEpochEvent(const EpochEvent& node,
                       cursiv::ValidationAcceptor& accept) const;

  void checkZuluEvent(const ZuluEvent& node,
                      cursiv::ValidationAcceptor& accept) const;

  void checkGlobalEventTriggeredEvent(const GlobalEventTriggeredEvent& node,
                                      cursiv::ValidationAcceptor& accept) const;

  void checkCallOperation(const CallOperation& node,
                          cursiv::ValidationAcceptor& accept) const;

  void checkSetProperty(const SetProperty& node,
                        cursiv::ValidationAcceptor& accept) const;

  void checkExecuteTask(const ExecuteTask& node,
                        cursiv::ValidationAcceptor& accept) const;

  void checkTransfer(const Transfer& node,
                     cursiv::ValidationAcceptor& accept) const;
};

template <typename Services>
void registerXsmpsedValidationChecks(Services& services) {
  using Registry = cursiv::ValidationRegistry;
  auto& registry = *services.validationRegistry;
  auto& validator = *services.validator;
  registry.registerChecks({
      Registry::makeValidationCheck<&XsmpsedValidator::checkPath>(validator),
      Registry::makeValidationCheck<&XsmpsedValidator::checkIntValue>(
          validator),
      Registry::makeValidationCheck<&XsmpsedValidator::checkFloatValue>(
          validator),
      Registry::makeValidationCheck<&XsmpsedValidator::checkChar8Value>(
          validator),
      Registry::makeValidationCheck<&XsmpsedValidator::checkInt32Parameter>(
          validator),
      Registry::makeValidationCheck<&XsmpsedValidator::checkStringParameter>(
          validator),
      Registry::makeValidationCheck<&XsmpsedValidator::checkSchedule>(
          validator),
      Registry::makeValidationCheck<&XsmpsedValidator::checkTask>(validator),
      Registry::makeValidationCheck<&XsmpsedValidator::checkMissionEvent>(
          validator),
      Registry::makeValidationCheck<&XsmpsedValidator::checkSimulationEvent>(
          validator),
      Registry::makeValidationCheck<&XsmpsedValidator::checkEpochEvent>(
          validator),
      Registry::makeValidationCheck<&XsmpsedValidator::checkZuluEvent>(
          validator),
      Registry::makeValidationCheck<
          &XsmpsedValidator::checkGlobalEventTriggeredEvent>(validator),
      Registry::makeValidationCheck<&XsmpsedValidator::checkCallOperation>(
          validator),
      Registry::makeValidationCheck<&XsmpsedValidator::checkSetProperty>(
          validator),
      Registry::makeValidationCheck<&XsmpsedValidator::checkExecuteTask>(
          validator),
      Registry::makeValidationCheck<&XsmpsedValidator::checkTransfer>(
          validator),
  });
}

}  // namespace xsmp
