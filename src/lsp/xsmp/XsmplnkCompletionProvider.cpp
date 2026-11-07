#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/XsmplnkCompletionProvider.hpp>

namespace xsmp {

// A statement stands where the grammar reads an element of a component link
// base, and at the root where it reads a path outside a link.
bool XsmplnkCompletionProvider::atStatement(const Context& context) const {
  return writes(context, &cursiv::ast_type_tag<ComponentLinkBase>, "elements")
         || (writes(context, &cursiv::ast_type_tag<Path>, "absolute")
             && !cursiv::container_of_type<Link>(context.node()));
}

void XsmplnkCompletionProvider::completeStatement(const Context& context,
                                                  const cursiv::AstNode* block,
                                                  Accept accept) const {
  if (!cursiv::ast_cast<ComponentLinkBase>(block)) {
    offerSnippet(context, accept, "Root Component Link Base", "/\n{\n\t$0\n}",
                 "Root Component Link Base");
    return;
  }
  offerSnippet(context, accept, "Component Link Base", "${1:path}\n{\n\t$0\n}",
               "Nested Component Link Base");
  const auto relative = relativeComponents(componentOf(block), true);
  for (const auto& child : relative)
    if (child.path != "."
        && !offerValue(context, accept, child.path, child.path + "\n{\n\t$0\n}",
                       "Nested Component Link Base", child.node))
      return;
  offerInterfaceLinks(context, accept, relative);
}

void XsmplnkCompletionProvider::completeKeyword(const Context& context,
                                                const cursiv::AstNode* block,
                                                std::string_view keyword,
                                                Accept accept) const {
  const auto offer = [&](std::string_view label, std::string body) {
    offerSnippet(context, accept, std::string(label), std::move(body),
                 std::string(label));
  };
  if (keyword == "link" && !context.node()) {
    const auto assemblies =
        names(context, &cursiv::ast_type_tag<LinkBase>, "assembly");
    offerSnippet(
        context, accept, "Link Base",
        "link ${1:Name}"
            + (assemblies.empty() ? std::string{}
                                  : " for " + choice(2, assemblies, "Assembly"))
            + "\n$0",
        "Link Base Definition");
  }

  if (!cursiv::ast_cast<ComponentLinkBase>(block)) return;
  const auto links = [&](bool events) {
    return linkEnds(relativeComponents(componentOf(block), true), events,
                    "owner", "client");
  };
  if (keyword == "event") offer("Event Link", "event link " + links(true));
  if (keyword == "field") offer("Field Link", "field link " + links(false));
  if (keyword == "interface")
    offer("Interface Link",
          "interface link ${1:sourcePath} -> "
          "${2:client}${3::${4:backReference}}");
}

}  // namespace xsmp
