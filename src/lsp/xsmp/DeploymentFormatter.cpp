#include <concepts>
#include <xsmp/DeploymentFormatter.hpp>

namespace xsmp {

DeploymentFormatter::DeploymentFormatter(const cursiv::CoreServices& services)
    : AbstractFormatter(services) {
  on<LocalNamedReference>(
      &DeploymentFormatter::formatUnsafe<LocalNamedReference>);
  on<ConcretePathNamedSegment>(
      &DeploymentFormatter::formatUnsafe<ConcretePathNamedSegment>);
  on<Path>(&DeploymentFormatter::formatPath);
  on<PathMember>(&DeploymentFormatter::formatPathMember);
  on<PathIndex>(&DeploymentFormatter::formatPathIndex);
  on<StringParameter>(&DeploymentFormatter::formatParameter<StringParameter>);
  on<Int32Parameter>(&DeploymentFormatter::formatParameter<Int32Parameter>);
  on<StringArgument>(&DeploymentFormatter::formatAssigned<StringArgument>);
  on<Int32Argument>(&DeploymentFormatter::formatAssigned<Int32Argument>);
  on<ParameterValue>(&DeploymentFormatter::formatAssigned<ParameterValue>);
  on<FieldValue>(&DeploymentFormatter::formatAssigned<FieldValue>);
  on<EventLink>(&DeploymentFormatter::formatLink<EventLink>);
  on<FieldLink>(&DeploymentFormatter::formatLink<FieldLink>);
  on<InterfaceLink>(&DeploymentFormatter::formatLink<InterfaceLink>);
  on<ArrayValue>(&DeploymentFormatter::formatArrayValue);
  on<StructureValue>(&DeploymentFormatter::formatStructureValue);
}

template <class T>
void DeploymentFormatter::formatUnsafe(cursiv::FormattingBuilder& builder,
                                       const T* node) const {
  builder.getNodeFormatter(node).keyword("unsafe").append(oneSpace);
}

template <class T>
void DeploymentFormatter::formatParameter(cursiv::FormattingBuilder& builder,
                                          const T* node) const {
  const auto f = builder.getNodeFormatter(node);
  formatAnnotation(f);
  formatAssignment(f);
}

template <class T>
void DeploymentFormatter::formatAssigned(cursiv::FormattingBuilder& builder,
                                         const T* node) const {
  formatAssignment(builder.getNodeFormatter(node));
}

template <class T>
void DeploymentFormatter::formatLink(cursiv::FormattingBuilder& builder,
                                     const T* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keywords("event", "field", "interface", "link").append(oneSpace);
  f.keyword("->").surround(oneSpace);
  // The back reference of `client:back`.
  if constexpr (std::same_as<T, InterfaceLink>)
    f.keywords(":").surround(noSpace);
}

void DeploymentFormatter::formatPath(cursiv::FormattingBuilder& builder,
                                     const Path* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("unsafe").append(oneSpace);
  if (node->absolute) f.keyword("/").append(noSpace);
  f.keywords("..", ".").surround(noSpace);
}

void DeploymentFormatter::formatPathMember(cursiv::FormattingBuilder& builder,
                                           const PathMember* node) const {
  builder.getNodeFormatter(node).property<&PathMember::separator>().surround(
      noSpace);
}

void DeploymentFormatter::formatPathIndex(cursiv::FormattingBuilder& builder,
                                          const PathIndex* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("[").surround(noSpace);
  f.keyword("]").prepend(noSpace);
}

void DeploymentFormatter::formatArrayValue(cursiv::FormattingBuilder& builder,
                                           const ArrayValue* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("[").append(noSpace);
  formatCommas(f);
  f.keyword("]").prepend(noSpace);
  // `[2: a, b]`, or `[2:]` with no item after it.
  if (node->startIndex)
    f.keyword(":").prepend(noSpace).append(
        node->elements.empty()
            ? static_cast<cursiv::FormattingAction>(noSpace)
            : static_cast<cursiv::FormattingAction>(oneSpace));
}

void DeploymentFormatter::formatStructureValue(
    cursiv::FormattingBuilder& builder, const StructureValue* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("{").append(oneSpace);
  formatCommas(f);
  f.keyword("}").prepend(oneSpace);
}

}  // namespace xsmp
