#include <algorithm>
#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/utils/TreeUtils.hpp>
#include <xsmp/validation/XsmplnkValidator.hpp>

namespace xsmp {

void XsmplnkValidator::checkLinkBase(const LinkBase& node,
                                     cursiv::ValidationAcceptor& accept) const {
  checkReferenceBounds(nullptr, &node, accept);
  if (node.assembly.refText.empty())
    for (auto* child : descendantsOf(node))
      if (cursiv::ast_cast<PatternPathNamedSegment>(child)) {
        accept
            .error(node,
                   "A Link Base using templated paths shall declare an "
                   "Assembly anchor with 'for <Assembly>'.")
            .keyword("link");
        break;
      }
  checkDocument(node, accept);
  if (node.elements.empty())
    accept
        .error(node,
               "A Link Base shall contain at least one Component Link Base.")
        .keyword("link");
}

void XsmplnkValidator::checkComponentLinkBase(
    const ComponentLinkBase& node, cursiv::ValidationAcceptor& accept) const {
  if (std::ranges::none_of(node.elements, [](auto* element) {
        return cursiv::ast_cast<Link>(element);
      }))
    accept.error(node, "A Component Link Base shall contain at least one Link.")
        .property<&ComponentLinkBase::name>();
}

void XsmplnkValidator::checkInterfaceLink(
    const InterfaceLink& node, cursiv::ValidationAcceptor& accept) const {
  checkInterface(node, accept);
}

void XsmplnkValidator::checkEventLink(
    const EventLink& node, cursiv::ValidationAcceptor& accept) const {
  checkEventConnection(node, accept);
}

void XsmplnkValidator::checkFieldLink(
    const FieldLink& node, cursiv::ValidationAcceptor& accept) const {
  checkFieldConnection(node, node.ownerPath, node.clientPath, accept);
}

}  // namespace xsmp
