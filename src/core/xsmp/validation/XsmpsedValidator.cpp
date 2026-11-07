#include <algorithm>
#include <string>
#include <unordered_set>
#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/solver/Solver.hpp>
#include <xsmp/utils/TreeUtils.hpp>
#include <xsmp/utils/XsmpUtils.hpp>
#include <xsmp/validation/XsmpsedValidator.hpp>

namespace xsmp {

namespace {

// The rules every kind of Event shares. An Event that repeats needs a positive
// cycle time, or for `simulationTime` -1 ns, which stops it after one run
// (ECSS-E-ST-40-07 scheduler requirements). An invalid one is reported as such.
void checkEvent(const Event& node, bool simulationTime,
                cursiv::ValidationAcceptor& accept) {
  checkDuration(node, node.cycleTime, "CycleTime", accept);
  if (node.repeatCount.value_or(0) < -1)
    accept
        .error(node,
               "RepeatCount shall be -1 for infinity or a non-negative number.")
        .range(rangeForFeature(node, "repeatCount"));
  if (const auto cycle = durationNanoseconds(node.cycleTime);
      cycle && node.repeatCount.value_or(0) != 0 && *cycle <= 0
      && !(simulationTime && *cycle == -1))
    accept.error(node, "A cyclic Event shall have a positive CycleTime.")
        .range(rangeForFeature(node, "cycleTime"));
  if (node.task.get()
      && cursiv::container_of_type<Schedule>(&node)
             != cursiv::container_of_type<Schedule>(node.task.get()))
    accept.error(node,
                 "An Event shall be associated with a Task defined in the "
                 "same Schedule.");
}

}  // namespace

void XsmpsedValidator::checkSchedule(const Schedule& node,
                                     cursiv::ValidationAcceptor& accept) const {
  auto used = usedParameters(node);
  // Paths read the first String8 parameter as their root, and all of them when
  // a path is relative.
  auto relative = false;
  const auto usesRelative = [](const Path* path) {
    return path && !path->absolute;
  };
  for (auto* item : node.elements)
    if (auto* task = cursiv::ast_cast<Task>(item))
      for (auto* activity : task->elements) {
        if (auto* trigger = cursiv::ast_cast<Trigger>(activity))
          relative |= usesRelative(trigger->entryPoint);
        if (auto* transfer = cursiv::ast_cast<Transfer>(activity))
          relative |= usesRelative(transfer->outputFieldPath)
                      || usesRelative(transfer->inputFieldPath);
        if (auto* property = cursiv::ast_cast<SetProperty>(activity))
          relative |= usesRelative(property->propertyPath);
        if (auto* call = cursiv::ast_cast<CallOperation>(activity))
          relative |= usesRelative(call->operationPath);
        if (auto* execute = cursiv::ast_cast<ExecuteTask>(activity))
          relative |= usesRelative(execute->root);
      }

  auto rootSeen = false;
  for (auto* parameter : node.parameters)
    if (cursiv::ast_cast<StringParameter>(parameter)) {
      if (relative || !rootSeen) used.insert(parameter->name);
      rootSeen = true;
    }

  checkUnusedParameters(node.parameters, used, accept);
  checkDocument(node, accept);
  checkDate(node, node.epochTime, "EpochTime", accept);
  checkDate(node, node.missionStart, "MissionStart", accept);

  if (std::ranges::none_of(node.parameters, [](auto* parameter) {
        return cursiv::ast_cast<StringParameter>(parameter);
      }))
    accept
        .error(node,
               "A Schedule shall declare at least one String8 Template "
               "Argument for the root path.")
        .keyword("schedule");
  checkDuplicatedParameters(node.parameters, accept);

  std::unordered_set<std::string_view> tasks;
  for (const auto* item : node.elements)
    if (const auto* task = cursiv::ast_cast<Task>(item);
        task && !tasks.insert(task->name).second)
      accept.error(*task, "Duplicated task name.")
          .range(rangeForFeature(*task, "name"));
}

void XsmpsedValidator::checkTask(const Task& node,
                                 cursiv::ValidationAcceptor& accept) const {
  checkInstanceName(node, "enclosing Schedule", accept);
}

void XsmpsedValidator::checkMissionEvent(
    const MissionEvent& node, cursiv::ValidationAcceptor& accept) const {
  checkEvent(node, false, accept);
  checkDuration(node, node.missionTime, "MissionTime", accept);
}

void XsmpsedValidator::checkSimulationEvent(
    const SimulationEvent& node, cursiv::ValidationAcceptor& accept) const {
  checkEvent(node, true, accept);
  checkDuration(node, node.simulationTime, "SimulationTime", accept);
}

void XsmpsedValidator::checkEpochEvent(
    const EpochEvent& node, cursiv::ValidationAcceptor& accept) const {
  checkEvent(node, false, accept);
  checkDate(node, node.epochTime, "EpochTime", accept);
}

void XsmpsedValidator::checkZuluEvent(
    const ZuluEvent& node, cursiv::ValidationAcceptor& accept) const {
  checkEvent(node, false, accept);
  checkDate(node, node.zuluTime, "ZuluTime", accept);
}

void XsmpsedValidator::checkGlobalEventTriggeredEvent(
    const GlobalEventTriggeredEvent& node,
    cursiv::ValidationAcceptor& accept) const {
  // Without a time kind, it runs on simulation time.
  checkEvent(node, node.timeKind.empty() || node.timeKind == "simulation",
             accept);
  checkDuration(node, node.delay, "Delay", accept);
}

void XsmpsedValidator::checkCallOperation(
    const CallOperation& node, cursiv::ValidationAcceptor& accept) const {
  if (node.operationPath && !node.operationPath->unsafe)
    checkOperationParameters(
        node, cursiv::ast_cast<Operation>(pathTarget(node.operationPath)),
        node.parameters, accept);
  checkDuplicatedCallParameters(node.parameters, accept);
}

void XsmpsedValidator::checkSetProperty(
    const SetProperty& node, cursiv::ValidationAcceptor& accept) const {
  if (node.propertyPath && !node.propertyPath->unsafe)
    checkPropertyAssignment(
        node, "propertyPath",
        cursiv::ast_cast<Property>(pathTarget(node.propertyPath)), node.value,
        accept);
}

void XsmpsedValidator::checkExecuteTask(
    const ExecuteTask& node, cursiv::ValidationAcceptor& accept) const {
  checkTemplateArguments(node.parameter, accept);
  if (!node.root || node.root->unsafe || !node.task.get()) return;

  // Twice: an Assembly Instance gives its root model, then that model's
  // Component.
  const auto* actual = cursiv::ast_cast<Component>(
      targetType(targetType(pathTarget(node.root))));

  const auto* task = node.task.get();
  const Component* expected = task->context.get<Component>();
  // An Assembly context stands for the Component of its root model.
  if (auto* assembly = task->context.get<Assembly>())
    expected = cursiv::ast_cast<Component>(targetType(assembly->model));

  if (actual && expected && !isBaseOfComponent(expected, actual))
    accept
        .error(node,
               "The root path shall resolve to a Component compatible with "
               "the execution context of task "
                   + std::string(node.task.refText) + ".")
        .property<&ExecuteTask::root>();
}

void XsmpsedValidator::checkTransfer(const Transfer& node,
                                     cursiv::ValidationAcceptor& accept) const {
  checkFieldConnection(node, node.outputFieldPath, node.inputFieldPath, accept);
}

}  // namespace xsmp
