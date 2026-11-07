#include <xsmp/XsmpsedFormatter.hpp>

namespace xsmp {

XsmpsedFormatter::XsmpsedFormatter(const cursiv::CoreServices& services)
    : DeploymentFormatter(services) {
  on<Schedule>(&XsmpsedFormatter::formatSchedule);
  on<Task>(&XsmpsedFormatter::formatTask);
  on<Transfer>(&XsmpsedFormatter::formatTransfer);
  on<Trigger>(&XsmpsedFormatter::formatTrigger);
  on<ExecuteTask>(&XsmpsedFormatter::formatExecuteTask);
  on<EmitGlobalEvent>(&XsmpsedFormatter::formatEmitGlobalEvent);
  on<CallOperation>(&XsmpsedFormatter::formatCallOperation);
  on<SetProperty>(&XsmpsedFormatter::formatSetProperty);
  on<MissionEvent>(&XsmpsedFormatter::formatEvent<MissionEvent>);
  on<EpochEvent>(&XsmpsedFormatter::formatEvent<EpochEvent>);
  on<SimulationEvent>(&XsmpsedFormatter::formatEvent<SimulationEvent>);
  on<ZuluEvent>(&XsmpsedFormatter::formatEvent<ZuluEvent>);
  on<GlobalEventTriggeredEvent>(
      &XsmpsedFormatter::formatEvent<GlobalEventTriggeredEvent>);
}

template <class T>
void XsmpsedFormatter::formatEvent(cursiv::FormattingBuilder& builder,
                                   const T* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("event").append(oneSpace);
  f.keywords("mission", "epoch", "simulation", "zulu", "cycle", "repeat", "on",
             "until", "using", "delay")
      .surround(oneSpace);
}

void XsmpsedFormatter::formatSchedule(cursiv::FormattingBuilder& builder,
                                      const Schedule* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("schedule").prepend(noIndent).append(oneSpace);
  formatAngles(f, true);
  f.property<&Schedule::name>().prepend(oneSpace);
  f.keywords("epoch", "mission").surround(oneSpace);
  for (const auto* element : node->elements) f.node(*element).prepend(noIndent);
}

void XsmpsedFormatter::formatTask(cursiv::FormattingBuilder& builder,
                                  const Task* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("task").append(oneSpace);
  f.keyword("on").surround(oneSpace);
  formatBody(f);
}

void XsmpsedFormatter::formatTransfer(cursiv::FormattingBuilder& builder,
                                      const Transfer* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("transfer").append(oneSpace);
  f.keyword("->").surround(oneSpace);
}

void XsmpsedFormatter::formatTrigger(cursiv::FormattingBuilder& builder,
                                     const Trigger* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("trig").append(oneSpace);
}

void XsmpsedFormatter::formatExecuteTask(cursiv::FormattingBuilder& builder,
                                         const ExecuteTask* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("execute").append(oneSpace);
  formatAngles(f, false);
  f.keyword("at").surround(oneSpace);
}

void XsmpsedFormatter::formatEmitGlobalEvent(
    cursiv::FormattingBuilder& builder, const EmitGlobalEvent* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keywords("async", "emit").append(oneSpace);
}

void XsmpsedFormatter::formatCallOperation(cursiv::FormattingBuilder& builder,
                                           const CallOperation* node) const {
  const auto f = builder.getNodeFormatter(node);
  formatCall(f);
}

void XsmpsedFormatter::formatSetProperty(cursiv::FormattingBuilder& builder,
                                         const SetProperty* node) const {
  const auto f = builder.getNodeFormatter(node);
  formatProperty(f);
}

}  // namespace xsmp
