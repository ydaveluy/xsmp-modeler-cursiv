#include <algorithm>
#include <ranges>
#include <xsmp/DeploymentSemantics.hpp>

namespace xsmp {

bool validIdentifier(std::string_view name) {
  if (name.empty()) return false;
  const auto letter = [](char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
  };
  if (!letter(name.front())) return false;
  return std::ranges::all_of(name.substr(1), [&](char c) {
    return letter(c) || (c >= '0' && c <= '9') || c == '_';
  });
}

void bindParameters(Bindings& out,
                    const std::vector<TemplateParameter*>& parameters) {
  for (auto* parameter : parameters) {
    if (auto* text = cursiv::ast_cast<StringParameter>(parameter);
        text && text->value.data())
      out[parameter->name] = text->value;
    if (auto* integer = cursiv::ast_cast<Int32Parameter>(parameter);
        integer && integer->value)
      out[parameter->name] = std::to_string(*integer->value);
  }
}

void bindInstance(Bindings& out, const AssemblyInstance& instance) {
  if (auto* assembly = instance.assembly.get())
    bindParameters(out, assembly->parameters);
  for (auto* argument : instance.arguments) {
    if (auto* text = cursiv::ast_cast<StringArgument>(argument))
      out[argument->parameter.refText] = text->value;
    if (auto* integer = cursiv::ast_cast<Int32Argument>(argument))
      out[argument->parameter.refText] = std::to_string(integer->value);
  }
}

Bindings bindingsFor(const AstNode* node) {
  Bindings out;
  std::vector<const AstNode*> ancestors;
  for (auto* current = node; current; current = current->parent)
    ancestors.push_back(current);
  for (auto current : std::views::reverse(ancestors)) {
    if (auto* assembly = cursiv::ast_cast<Assembly>(current))
      bindParameters(out, assembly->parameters);
    if (auto* links = cursiv::ast_cast<LinkBase>(current);
        links && links->assembly.get())
      bindParameters(out, links->assembly.get()->parameters);
    if (auto* configuration = cursiv::ast_cast<ComponentConfiguration>(current);
        configuration && configuration->context.get<Assembly>())
      bindParameters(out, configuration->context.get<Assembly>()->parameters);
    if (auto* task = cursiv::ast_cast<Task>(current);
        task && task->context.get<Assembly>()) {
      // A task inherits the parameters of its assembly, but those of its
      // schedule keep their value.
      Bindings inherited;
      bindParameters(inherited, task->context.get<Assembly>()->parameters);
      for (auto& [name, value] : inherited)
        out.try_emplace(name, std::move(value));
    }
    if (auto* schedule = cursiv::ast_cast<Schedule>(current))
      bindParameters(out, schedule->parameters);
    if (auto* instance = cursiv::ast_cast<AssemblyInstance>(current))
      bindInstance(out, *instance);
  }
  return out;
}

std::optional<std::string> expanded(std::string_view pattern,
                                    const Bindings& bindings) {
  std::string result;
  while (!pattern.empty()) {
    const auto open = pattern.find('{');
    if (open == std::string_view::npos) {
      result += pattern;
      break;
    }
    result += pattern.substr(0, open);
    pattern.remove_prefix(open + 1);
    const auto close = pattern.find('}');
    if (close == std::string_view::npos) return std::nullopt;
    const auto found = bindings.find(pattern.substr(0, close));
    if (found == bindings.end()) return std::nullopt;
    result += found->second;
    pattern.remove_prefix(close + 1);
  }
  return result;
}

std::unordered_set<std::string_view> templateParameterNames(
    const AstNode& node) {
  std::unordered_set<std::string_view> names;
  const auto add = [&](auto parameters) {
    for (const auto* parameter : parameters) names.insert(parameter->name);
  };
  for (auto* current = &node; current; current = current->parent) {
    if (auto* assembly = cursiv::ast_cast<Assembly>(current))
      add(assembly->parameters);
    if (auto* schedule = cursiv::ast_cast<Schedule>(current))
      add(schedule->parameters);
    if (auto* links = cursiv::ast_cast<LinkBase>(current);
        links && links->assembly.get())
      add(links->assembly.get()->parameters);
    if (auto* configuration = cursiv::ast_cast<ComponentConfiguration>(current);
        configuration && configuration->context.get<Assembly>())
      add(configuration->context.get<Assembly>()->parameters);
    if (auto* task = cursiv::ast_cast<Task>(current);
        task && task->context.get<Assembly>())
      add(task->context.get<Assembly>()->parameters);
    // The own name of an instance is read outside it: the parameters of the
    // assembly it instantiates apply only below it.
    if (auto* instance = cursiv::ast_cast<AssemblyInstance>(current);
        instance && current != &node && instance->assembly.get())
      add(instance->assembly.get()->parameters);
  }
  return names;
}

}  // namespace xsmp
