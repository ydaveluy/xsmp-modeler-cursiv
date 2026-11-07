#include <cursiv/parser/Parser.hpp>
#include <cursiv/syntax-tree/AstReflection.hpp>
#include <utility>
#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/XsmpasbCompletionProvider.hpp>

namespace xsmp {

namespace {

const auto* const kModel = &cursiv::ast_type_tag<ModelInstance>;
const auto* const kInstance = &cursiv::ast_type_tag<AssemblyInstance>;

}  // namespace

bool XsmpasbCompletionProvider::atStatement(const Context& context) const {
  return writes(context, kModel, "elements")
         || writes(context,
                   &cursiv::ast_type_tag<AssemblyComponentConfiguration>,
                   "elements")
         || writes(context, &cursiv::ast_type_tag<Assembly>, "configurations");
}

void XsmpasbCompletionProvider::completeStatement(const Context& context,
                                                  const cursiv::AstNode* block,
                                                  Accept accept) const {
  const auto components = names(context, kModel, "implementation");
  if (!cursiv::ast_cast<ModelInstance>(block)) {
    if (cursiv::ast_cast<AssemblyComponentConfiguration>(block)) {
      offerFieldAssignments(context, accept, componentOf(block), "Field of ");
      offerInvocations(context, accept, componentOf(block), true);
    } else if (!block) {
      offerSnippet(context, accept, "Root Model Instance",
                   "${1:Root}: " + choice(2, components, "demo.Component")
                       + "\n{\n\t$0\n}",
                   "Root Model Instance");
    }
    return;
  }

  const auto assemblies = names(context, kInstance, "assembly");
  const auto configurations = names(context, kInstance, "configuration");
  const auto links = names(context, kInstance, "linkBase");
  // The `using` clauses, for what can be named, with tab stops from `index`.
  const auto usage = [&](unsigned index) {
    std::string suffix;
    if (!configurations.empty())
      suffix +=
          " using config " + choice(index++, configurations, "Configuration");
    if (!links.empty())
      suffix += " using link " + choice(index, links, "LinkBase");
    return suffix;
  };

  offerSnippet(context, accept, "Sub Model Instance",
               "${1:container} += ${2:Child}: "
                   + choice(3, components, "demo.Component"),
               "Sub Model Instance");
  offerSnippet(context, accept, "Sub Assembly Instance",
               "${1:container} += ${2:Child}: "
                   + choice(3, assemblies, "Assembly") + usage(4),
               "Sub Assembly Instance");

  offerFieldAssignments(context, accept, block, "Field of ");
  for (const auto* member : members(block))
    if (const auto* container = cursiv::ast_cast<Container>(member);
        container && !container->name.empty()) {
      const std::string name(container->name);
      const auto* child =
          container->defaultComponent
              ? container->defaultComponent->get()
              : cursiv::ast_cast<Component>(container->type.get());
      if (!child) continue;

      // A container with a default component needs no type after the name.
      auto body = name + " += " + placeholder(1, child->name);
      if (!container->defaultComponent) body += ": " + qualifiedName(*child);
      if (!offerValue(context, accept,
                      name + " += " + std::string(child->name)
                          + (container->defaultComponent
                                 ? ""
                                 : ": " + qualifiedName(*child)),
                      body, "Sub Model Instance", container))
        return;
      if (!assemblies.empty()
          && !offerValue(
              context, accept,
              name + " += " + std::string(child->name) + ": Assembly",
              name + " += " + placeholder(1, child->name) + ": "
                  + choice(2, assemblies, "Assembly") + usage(3),
              "Sub Assembly Instance", container))
        return;
    }

  offerInterfaceLinks(context, accept, relativeComponents(block, false));
}

void XsmpasbCompletionProvider::completeKeyword(const Context& context,
                                                const cursiv::AstNode* block,
                                                std::string_view keyword,
                                                Accept accept) const {
  const auto offer = [&](std::string_view label, std::string_view body,
                         std::string_view detail) {
    offerSnippet(context, accept, std::string(label), std::string(body),
                 std::string(detail));
  };

  if (keyword == "assembly")
    offer("Assembly", "assembly ${1:Name}\n$0", "Assembly Definition");
  if (keyword == "configure")
    offer("Configure Instance", "configure ${1:path}\n{\n\t$0\n}",
          "Component Configuration");

  if (cursiv::ast_cast<AssemblyComponentConfiguration>(block)) {
    if (keyword == "subscribe")
      offer("Subscribe", "subscribe ${1:entryPoint} -> \"${2:GlobalEvent}\"",
            "Global Event Subscription");
    if (keyword == "property")
      offer("Property Value", "property ${1:name} = ${2:value}",
            "Property Value");
    if (keyword == "call")
      offer("Operation Call", "call ${1:name}($0)", "Operation Call");
  }

  if (cursiv::ast_cast<ModelInstance>(block)) {
    const auto links = [&](bool events) {
      return linkEnds(relativeComponents(block, false), events, "owner",
                      "client");
    };
    if (keyword == "event")
      offer("Event Link", "event link " + links(true), "Event Link");
    if (keyword == "field")
      offer("Field Link", "field link " + links(false), "Field Link");
    if (keyword == "interface")
      offer("Interface Link",
            "interface link ${1:sourcePath} -> "
            "${2:client}${3::${4:backReference}}",
            "Interface Link");
  }
}

}  // namespace xsmp
