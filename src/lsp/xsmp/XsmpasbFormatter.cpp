#include <xsmp/XsmpasbFormatter.hpp>

namespace xsmp {

XsmpasbFormatter::XsmpasbFormatter(const cursiv::CoreServices& services)
    : DeploymentFormatter(services) {
  on<Assembly>(&XsmpasbFormatter::formatAssembly);
  on<AssemblyComponentConfiguration>(
      &XsmpasbFormatter::formatAssemblyComponentConfiguration);
  on<ModelInstance>(&XsmpasbFormatter::formatModelInstance);
  on<AssemblyInstance>(&XsmpasbFormatter::formatAssemblyInstance);
  on<SubInstance>(&XsmpasbFormatter::formatSubInstance);
  on<GlobalEventHandler>(&XsmpasbFormatter::formatGlobalEventHandler);
  on<OperationCall>(&XsmpasbFormatter::formatOperationCall);
  on<PropertyValue>(&XsmpasbFormatter::formatPropertyValue);
}

void XsmpasbFormatter::formatAssembly(cursiv::FormattingBuilder& builder,
                                      const Assembly* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("assembly").prepend(noIndent).append(oneSpace);
  formatAngles(f, true);
  f.property<&Assembly::name>().prepend(oneSpace).append(
      newLine({.allowMore = true}));
  for (const auto* configuration : node->configurations)
    f.node(*configuration).prepend(noIndent);
  if (node->model) f.node(*node->model).prepend(noIndent);
}

void XsmpasbFormatter::formatAssemblyComponentConfiguration(
    cursiv::FormattingBuilder& builder,
    const AssemblyComponentConfiguration* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("configure").append(oneSpace);
  formatBody(f);
}

void XsmpasbFormatter::formatModelInstance(cursiv::FormattingBuilder& builder,
                                           const ModelInstance* node) const {
  const auto f = builder.getNodeFormatter(node);
  formatAnnotation(f);
  formatBody(f);
}

void XsmpasbFormatter::formatAssemblyInstance(
    cursiv::FormattingBuilder& builder, const AssemblyInstance* node) const {
  const auto f = builder.getNodeFormatter(node);
  formatAnnotation(f);
  formatAngles(f, false);
  f.keywords("using").surround(oneSpace);
  f.keywords("config", "link").append(oneSpace);
  formatBody(f);
}

void XsmpasbFormatter::formatSubInstance(cursiv::FormattingBuilder& builder,
                                         const SubInstance* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("+=").surround(oneSpace);
}

void XsmpasbFormatter::formatGlobalEventHandler(
    cursiv::FormattingBuilder& builder, const GlobalEventHandler* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("subscribe").append(oneSpace);
  f.keyword("->").surround(oneSpace);
}

void XsmpasbFormatter::formatOperationCall(cursiv::FormattingBuilder& builder,
                                           const OperationCall* node) const {
  const auto f = builder.getNodeFormatter(node);
  formatCall(f);
}

void XsmpasbFormatter::formatPropertyValue(cursiv::FormattingBuilder& builder,
                                           const PropertyValue* node) const {
  const auto f = builder.getNodeFormatter(node);
  formatProperty(f);
}

}  // namespace xsmp
