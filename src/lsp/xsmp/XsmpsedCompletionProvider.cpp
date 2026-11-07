#include <array>
#include <tuple>
#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/XsmpsedCompletionProvider.hpp>

namespace xsmp {

bool XsmpsedCompletionProvider::atStatement(const Context& context) const {
  return writes(context, &cursiv::ast_type_tag<Task>, "elements");
}

void XsmpsedCompletionProvider::completeStatement(const Context& context,
                                                  const cursiv::AstNode* block,
                                                  Accept accept) const {
  const auto* task = cursiv::ast_cast<Task>(block);
  if (!task) return;
  const auto* component = componentOf(block);
  offerInvocations(context, accept, component, false);
  if (const auto* schedule = cursiv::container_of_type<Schedule>(task->parent))
    for (const auto* node : schedule->elements)
      if (const auto* other = cursiv::ast_cast<Task>(node);
          other && other != task
          && !offerValue(context, accept, "execute " + std::string(other->name),
                         "execute " + executeText(*other),
                         "Execute task " + std::string(other->name) + ".",
                         other))
        return;
}

void XsmpsedCompletionProvider::completeKeyword(const Context& context,
                                                const cursiv::AstNode* block,
                                                std::string_view keyword,
                                                Accept accept) const {
  const auto offer = [&](std::string_view label, std::string body,
                         std::string_view detail) {
    offerSnippet(context, accept, std::string(label), std::move(body),
                 std::string(detail));
  };
  if (keyword == "schedule")
    offer("Schedule", "schedule ${1:Name}\n$0", "Schedule Definition");
  if (keyword == "task")
    offer("Task", "task ${1:Name}${2: on demo.Component}\n{\n\t$0\n}",
          "Task Definition");
  if (keyword == "event") {
    const auto tasks = choice(
        1, names(context, &cursiv::ast_type_tag<ExecuteTask>, "task"), "Task");
    // A label such as `Event Epoch` gives the detail `Epoch Event`.
    for (const auto& [label, key, value] : std::array<
             std::tuple<std::string_view, std::string_view, std::string_view>,
             4>{{{"Event Epoch", "epoch", "2025-01-01T00:00:00Z"},
                 {"Event Mission", "mission", "PT10S"},
                 {"Event Simulation", "simulation", "PT10S"},
                 {"Event Zulu", "zulu", "2025-01-01T00:00:00Z"}}})
      offer(label,
            "event " + tasks + " " + std::string(key) + " \""
                + placeholder(2, value) + "\"",
            std::string(label.substr(6)) + " Event");
    offer("Event Global Trigger",
          "event " + tasks
              + " on \"${2:PlatformReady}\" using "
                "${3|mission,epoch,simulation,zulu|}",
          "Global Event Triggered Event");
  }
  if (keyword == "call")
    offer("Operation Call", "call ${1:operation}($0)", "Operation Call");
  if (keyword == "property")
    offer("Property Value", "property ${1:path} = ${2:value}",
          "Property Value");
  if (keyword == "transfer")
    offer("Field Transfer",
          "transfer "
              + linkEnds(relativeComponents(componentOf(block), false), false,
                         "output", "input"),
          "Field Transfer");
  if (keyword == "trig")
    offer("Entry Point Trigger", "trig ${1:path}", "Entry Point Trigger");
  if (keyword == "execute")
    offer("Execute Task", "execute ${1:Task} at ${2:path}", "Execute Task");
  if (keyword == "emit")
    offer("Emit Global Event", "emit \"${1:GlobalEvent}\"",
          "Emit Global Event");
}

}  // namespace xsmp
