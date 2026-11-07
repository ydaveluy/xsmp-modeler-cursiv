#include <xsmp/XsmplnkFormatter.hpp>

namespace xsmp {

XsmplnkFormatter::XsmplnkFormatter(const cursiv::CoreServices& services)
    : DeploymentFormatter(services) {
  on<LinkBase>(&XsmplnkFormatter::formatLinkBase);
  on<ComponentLinkBase>(&XsmplnkFormatter::formatComponentLinkBase);
}

void XsmplnkFormatter::formatLinkBase(cursiv::FormattingBuilder& builder,
                                      const LinkBase* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("link").prepend(noIndent).append(oneSpace);
  f.keyword("for").surround(oneSpace);
  f.property<&LinkBase::name>().append(newLine({.allowMore = true}));
  for (const auto* element : node->elements) f.node(*element).prepend(noIndent);
}

void XsmplnkFormatter::formatComponentLinkBase(
    cursiv::FormattingBuilder& builder, const ComponentLinkBase* node) const {
  const auto f = builder.getNodeFormatter(node);
  formatBody(f);
}

}  // namespace xsmp
