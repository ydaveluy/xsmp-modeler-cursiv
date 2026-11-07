#pragma once

#include <concepts>
#include <xsmp/DeploymentGrammarCommon.hpp>

namespace xsmp {

// The schedule language (`.xsmpsed`): tasks, and the events that start them.
struct Xsmpsed : cursiv::extends<DeploymentRules> {
  template <class>
  struct override;

  // A path may be the root alone, so what follows it can start the next
  // statement: these words are read as a name only when a name goes on.
  template <class T>
    requires std::same_as<T, DeploymentRules::ValidID>

  struct override<T>
      : rule<ref<ID> | kw<"async"> | kw<"call"> | kw<"emit"> | kw<"execute">
             | kw<"property"> | kw<"transfer"> | kw<"trig">> {};

  // A task name may hold template placeholders.
  struct Name : rule<ref<ValidID> | ref<PATTERN>> {};

  // A task of this schedule, or `Schedule.Task` for one of another schedule.
  struct TaskQualifiedName : rule<some(ref<Name>, lit<".">)> {};

  struct CallOperation
      : rule<kw<"call">.doc<R"(`call <operation-path>(<parameter = value, ...>)`

Calls one operation on the current task execution context or on a child component reached through a typed path.

**Example**
```xsmp
call avionics.apply(requestedMode = demo.foundation.Mode.Nominal)
```)">() + assign<&xsmp::CallOperation::operationPath>(ref<Path>)
             + lit<"(">
             + many(
                 append<&xsmp::CallOperation::parameters>(ref<ParameterValue>),
                 lit<",">)
             + lit<")">> {};

  struct SetProperty
      : rule<kw<"property">.doc<R"(`property <property-path> = <value>`

Sets one writable property on the current task execution context or on a child component reached through a typed path.

**Example**
```xsmp
property mode = demo.foundation.Mode.Nominal
```)">() + assign<&xsmp::SetProperty::propertyPath>(ref<Path>)
             + lit<"="> + assign<&xsmp::SetProperty::value>(ref<SimpleValue>)> {
  };

  struct Transfer
      : rule<kw<"transfer">.doc<R"(`transfer <output-field> -> <input-field>`

Copies data from one output field to one compatible input field.

**Example**
```xsmp
transfer bus.outValue -> router.incomingBus
```)">() + assign<&xsmp::Transfer::outputFieldPath>(ref<Path>)
             + lit<"->"> + assign<&xsmp::Transfer::inputFieldPath>(ref<Path>)> {
  };

  struct Trigger : rule<kw<"trig">.doc<R"(`trig <entrypoint-path>`

Executes one entry point of the current task execution context or of a reachable child component.

**Example**
```xsmp
trig avionics.step
```)">() + assign<&xsmp::Trigger::entryPoint>(ref<Path>)> {};

  struct ExecuteTask
      : rule<
            kw<"execute">.doc<R"(`execute <task> [<argument, ...>] [at <root-path>]`

Executes another task, optionally with template arguments and with a different execution root.

**Example**
```xsmp
execute PowerBalance at avionics
```)">() + assign<&xsmp::ExecuteTask::task>(ref<TaskQualifiedName>)
            + option(lit<"<">
                     + some(append<&xsmp::ExecuteTask::parameter>(
                                ref<TemplateArgument>),
                            lit<",">)
                     + lit<">">)
            + option(kw<"at"> + assign<&xsmp::ExecuteTask::root>(ref<Path>))> {
  };

  struct EmitGlobalEvent
      : rule<option(flag<&xsmp::EmitGlobalEvent::asynchronous>(kw<"async">))
             + kw<"emit">.doc<R"(`emit "<event-name>"`

Emits a simulator-wide global event.
Prefix the activity with `async` when the emission should be modeled as asynchronous with respect to the current activity flow.

**Example**
```xsmp
async emit "PlatformReady"
```)">() + assign<&xsmp::EmitGlobalEvent::eventName>(ref<DecodedString>)> {};

  struct Activity
      : rule<ref<ExecuteTask> | ref<EmitGlobalEvent> | ref<CallOperation>
             | ref<SetProperty> | ref<Transfer> | ref<Trigger>> {};

  struct Task
      : rule<
            create<xsmp::Task>
            + kw<"task">.doc<R"(`task <name> [on <component-or-assembly>] { ... }`

Declares a reusable schedule task.
Attach an execution context component or assembly when the task should be validated against a specific root context and when typed hover/navigation is desired on activity paths.

**Example**
```xsmp
task Bootstrap on demo.orbit.OrbitalPlatform
{
    trig run
}
```)">() + assign<&xsmp::NamedElement::name>(ref<Name>)
            + option(kw<"on">
                     + assign<&xsmp::Task::context>(ref<QualifiedName>))
            + lit<"{">
            + many(append<&xsmp::Task::elements>(ref<Activity>)) + lit<"}">> {};

  // What every event ends with.
  static constexpr auto EventCycle =
      option(kw<"cycle"> + assign<&xsmp::Event::cycleTime>(ref<STRING>)
             + option(kw<"repeat">
                      + assign<&xsmp::Event::repeatCount>(ref<SignedInteger>)));

  struct MissionEvent
      : rule<
            create<xsmp::MissionEvent>
            + kw<"event">.doc<R"(`event <task> mission "<time>" [cycle "..." [repeat <n>]]`

Schedules a task at one mission-relative time.

**Example**
```xsmp
event Bootstrap mission "PT10S"
```)">() + assign<&xsmp::Event::task>(ref<TaskQualifiedName>)
            + kw<"mission">
            + assign<&xsmp::MissionEvent::missionTime>(ref<STRING>)
            + EventCycle> {};

  struct EpochEvent
      : rule<
            create<xsmp::EpochEvent>
            + kw<"event">.doc<R"(`event <task> epoch "<time>" [cycle "..." [repeat <n>]]`

Schedules a task at one absolute epoch timestamp.

**Example**
```xsmp
event Bootstrap epoch "2025-01-01T00:00:10Z"
```)">() + assign<&xsmp::Event::task>(ref<TaskQualifiedName>)
            + kw<"epoch"> + assign<&xsmp::EpochEvent::epochTime>(ref<STRING>)
            + EventCycle> {};

  struct SimulationEvent
      : rule<
            create<xsmp::SimulationEvent>
            + kw<"event">.doc<R"(`event <task> simulation "<time>" [cycle "..." [repeat <n>]]`

Schedules a task at one simulation-relative time.

**Example**
```xsmp
event Worker simulation "PT45S"
```)">() + assign<&xsmp::Event::task>(ref<TaskQualifiedName>)
            + kw<"simulation">
            + assign<&xsmp::SimulationEvent::simulationTime>(ref<STRING>)
            + EventCycle> {};

  struct ZuluEvent
      : rule<
            create<xsmp::ZuluEvent>
            + kw<"event">.doc<R"(`event <task> zulu "<time>" [cycle "..." [repeat <n>]]`

Schedules a task at one UTC timestamp.

**Example**
```xsmp
event Bootstrap zulu "2025-01-01T00:00:10Z"
```)">() + assign<&xsmp::Event::task>(ref<TaskQualifiedName>)
            + kw<"zulu">
            + assign<&xsmp::ZuluEvent::zuluTime>(ref<STRING>) + EventCycle> {};

  struct GlobalEventTriggeredEvent
      : rule<
            create<xsmp::GlobalEventTriggeredEvent>
            + kw<"event">.doc<R"(`event <task> on "<start>" [until "<stop>"] [using <time-kind>] [delay "..."] [cycle "..." [repeat <n>]]`

Starts a task when a global event is emitted.
Use `until` to stop listening, `delay` to offset execution and `cycle` / `repeat` to create repeated activations.

**Example**
```xsmp
event Observe on "PlatformReady" using mission delay "PT5S" cycle "PT30S" repeat 3
```)">() + assign<&xsmp::Event::task>(ref<TaskQualifiedName>)
            + kw<"on">
            + assign<&xsmp::GlobalEventTriggeredEvent::startEvent>(ref<DecodedString>)
            + option(kw<"until">
                     + assign<&xsmp::GlobalEventTriggeredEvent::stopEvent>(
                         ref<DecodedString>))
            + option(kw<"using">
                     + assign<&xsmp::GlobalEventTriggeredEvent::timeKind>(
                         kw<"mission"> | kw<"epoch"> | kw<"simulation">
                         | kw<"zulu">))
            + option(
                kw<"delay">
                + assign<&xsmp::GlobalEventTriggeredEvent::delay>(ref<STRING>))
            + EventCycle> {};

  struct Event : rule<ref<MissionEvent> | ref<EpochEvent> | ref<SimulationEvent>
                      | ref<ZuluEvent> | ref<GlobalEventTriggeredEvent>> {};

  struct Schedule
      : rule<
            create<xsmp::Schedule>
            + kw<"schedule">.doc<R"(`schedule [<parameter, ...>] <name> [epoch "..."] [mission "..."]`

Declares an XSMP schedule document.
A schedule contains tasks, events and optional template parameters used to specialize task roots and templated paths.

**Example**
```xsmp
schedule <Root = "platform"> MissionPlan epoch "2025-01-01T00:00:00Z"
task Main on demo.Platform
{
}
```)">()
            + option(lit<"<">
                     + some(append<&xsmp::Schedule::parameters>(
                                ref<TemplateParameter>),
                            lit<",">)
                     + lit<">">)
            + assign<&xsmp::NamedElement::name>(ref<ValidID>)
            + option(
                (kw<"epoch"> + assign<&xsmp::Schedule::epochTime>(ref<STRING>)
                 + option(kw<"mission">
                          + assign<&xsmp::Schedule::missionStart>(ref<STRING>)))
                | (kw<"mission">
                   + assign<&xsmp::Schedule::missionStart>(ref<STRING>)
                   + option(kw<"epoch">
                            + assign<&xsmp::Schedule::epochTime>(ref<STRING>))))
            + many(append<&xsmp::Schedule::elements>(ref<Task> | ref<Event>))> {
  };

  using entry = Schedule;
};

}  // namespace xsmp
