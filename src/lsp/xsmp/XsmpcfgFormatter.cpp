#include <xsmp/XsmpcfgFormatter.hpp>

namespace xsmp {

XsmpcfgFormatter::XsmpcfgFormatter(const cursiv::CoreServices& services)
    : DeploymentFormatter(services) {
  on<Configuration>(&XsmpcfgFormatter::formatConfiguration);
  on<ComponentConfiguration>(&XsmpcfgFormatter::formatComponentConfiguration);
  on<ConfigurationUsage>(&XsmpcfgFormatter::formatConfigurationUsage);
  on<CfgStructureFieldValue>(&XsmpcfgFormatter::formatCfgStructureFieldValue);
}

void XsmpcfgFormatter::formatConfiguration(cursiv::FormattingBuilder& builder,
                                           const Configuration* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("configuration").prepend(noIndent).append(oneSpace);
  f.property<&Configuration::name>().append(newLine({.allowMore = true}));
}

void XsmpcfgFormatter::formatComponentConfiguration(
    cursiv::FormattingBuilder& builder,
    const ComponentConfiguration* node) const {
  const auto f = builder.getNodeFormatter(node);
  if (cursiv::ast_cast<Configuration>(node->parent))
    f.property<&ComponentConfiguration::name>().prepend(
        newLine({.allowMore = true}));
  formatAnnotation(f);
  for (const auto* element : node->elements)
    f.node(*element).prepend(newLine({.allowMore = true}));
  formatBody(f);
}

void XsmpcfgFormatter::formatConfigurationUsage(
    cursiv::FormattingBuilder& builder, const ConfigurationUsage* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("include").append(oneSpace);
  f.keyword("at").surround(oneSpace);
}

void XsmpcfgFormatter::formatCfgStructureFieldValue(
    cursiv::FormattingBuilder& builder,
    const CfgStructureFieldValue* node) const {
  const auto f = builder.getNodeFormatter(node);
  formatAssignment(f);
}

}  // namespace xsmp
