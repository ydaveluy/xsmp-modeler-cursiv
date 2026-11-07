#include <xsmp/XsmpcatFormatter.hpp>

namespace xsmp {

template <class Rules>
void XsmpcatFormatter::formatTypeHead(const Rules& f) {
  f.template properties<&VisibilityElement::modifiers>().append(oneSpace);
  f.template property<&NamedElement::name>().prepend(oneSpace);
  f.keyword("extends").surround(oneSpace);
}

template <class Rules>
void XsmpcatFormatter::formatBody(const Rules& f) {
  auto open = f.keyword("{");
  auto close = f.keyword("}");
  open.prepend(newLine).append(newLine({.allowMore = true}));
  f.interior(open, close).prepend(indent({.allowMore = true}));
  close.prepend(newLine).append(
      newLine({.allowMore = true, .allowLess = true}));
}

void XsmpcatFormatter::formatAttribute(Builder& builder,
                                       const Attribute* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.property<&Attribute::type>().prepend(noSpace);
  f.keyword("(").surround(noSpace);
  f.keyword(")").prepend(noSpace);
  // What follows stays on the line it was written on.
  f.node(*node).append(fit(oneSpace, newLine));
}

void XsmpcatFormatter::formatParameter(Builder& builder,
                                       const Parameter* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.property<&Parameter::direction>().append(oneSpace);
  f.property<&NamedElement::name>().prepend(oneSpace);
  f.keyword("=").surround(oneSpace);
}

void XsmpcatFormatter::formatReturnParameter(
    Builder& builder, const ReturnParameter* node) const {
  builder.getNodeFormatter(node).property<&ReturnParameter::name>().surround(
      oneSpace);
}

template <class T, auto TypeMember>
void XsmpcatFormatter::formatMultipleMember(Builder& builder,
                                            const T* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.template property<TypeMember>().prepend(oneSpace);
  f.template property<&NamedElement::name>().prepend(oneSpace);
  f.template property<&NamedElementWithMultiplicity::optional_>().prepend(
      noSpace);
  f.template property<&NamedElementWithMultiplicity::multiplicity>().prepend(
      noSpace);
  f.keyword("=").surround(oneSpace);
}

void XsmpcatFormatter::formatMultiplicity(Builder& builder,
                                          const Multiplicity* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("[").append(noSpace);
  f.keyword("]").prepend(noSpace);
  f.keyword("...").surround(oneSpace);
}

void XsmpcatFormatter::formatEntryPoint(Builder& builder,
                                        const EntryPoint* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.property<&NamedElement::name>().prepend(oneSpace);
  formatBody(f);
  // One section a line: the name stays beside its `in` or `out`.
  f.properties<&EntryPoint::input>().prepend(oneSpace);
  f.properties<&EntryPoint::output>().prepend(oneSpace);
}

template <class T>
void XsmpcatFormatter::formatEventMember(Builder& builder,
                                         const T* node) const {
  builder.getNodeFormatter(node).template property<&T::type>().surround(
      oneSpace);
}

void XsmpcatFormatter::formatEnumerationLiteral(
    Builder& builder, const EnumerationLiteral* node) const {
  builder.getNodeFormatter(node).keyword("=").surround(oneSpace);
}

void XsmpcatFormatter::formatCatalogue(Builder& builder,
                                       const Catalogue* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("catalogue").prepend(noIndent);
  f.property<&NamedElement::name>().prepend(oneSpace).append(
      newLine({.allowMore = true}));
  f.properties<&Catalogue::elements>().prepend(noIndent);
}

void XsmpcatFormatter::formatNamespace(Builder& builder,
                                       const Namespace* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("namespace").append(oneSpace);
  f.keywords("::").surround(noSpace);
  formatBody(f);
}

template <class T>
void XsmpcatFormatter::formatTypedMember(Builder& builder,
                                         const T* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.template properties<&VisibilityElement::modifiers>().append(oneSpace);
  f.template property<&T::type>().surround(oneSpace);
  f.keyword("=").surround(oneSpace);
}

void XsmpcatFormatter::formatProperty(Builder& builder,
                                      const Property* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.properties<&VisibilityElement::modifiers>().append(oneSpace);
  f.property<&Property::type>().surround(oneSpace);
  f.keyword("get").surround(oneSpace);
  f.keyword("set").surround(oneSpace);
  f.keywords("throws").append(oneSpace);
  f.keywords(",").prepend(noSpace).append(oneSpace);
  f.keyword("->").surround(oneSpace);
}

void XsmpcatFormatter::formatOperation(Builder& builder,
                                       const Operation* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.properties<&VisibilityElement::modifiers>().append(oneSpace);
  f.keyword("void").surround(oneSpace);
  f.property<&Operation::returnParameter>().surround(oneSpace);
  f.property<&NamedElement::name>().prepend(oneSpace).append(noSpace);
  f.keyword("(").append(noSpace);
  f.keywords(",").prepend(noSpace).append(oneSpace);
  f.keyword(")").prepend(noSpace);
  f.keyword("throws").surround(oneSpace);
}

template <class T>
void XsmpcatFormatter::formatBlockType(Builder& builder, const T* node) const {
  const auto f = builder.getNodeFormatter(node);
  formatTypeHead(f);
  f.keyword("implements").surround(oneSpace);
  f.keywords(",").prepend(noSpace).append(oneSpace);
  formatBody(f);
}

template <class T>
void XsmpcatFormatter::formatLineType(Builder& builder, const T* node) const {
  const auto f = builder.getNodeFormatter(node);
  formatTypeHead(f);
  f.keyword("=").surround(oneSpace);
  f.keyword("[").surround(noSpace);
  f.keyword("]").prepend(noSpace);
  f.keyword("*").prepend(noSpace);
}

void XsmpcatFormatter::formatInteger(Builder& builder,
                                     const Integer* node) const {
  const auto f = builder.getNodeFormatter(node);
  formatTypeHead(f);
  f.keyword("in").surround(oneSpace);
  f.keyword("...").surround(oneSpace);
}

void XsmpcatFormatter::formatFloat(Builder& builder, const Float* node) const {
  const auto f = builder.getNodeFormatter(node);
  formatTypeHead(f);
  f.keyword("in").surround(oneSpace);
  f.property<&Float::range>().surround(oneSpace);
}

void XsmpcatFormatter::formatEnumeration(Builder& builder,
                                         const Enumeration* node) const {
  const auto f = builder.getNodeFormatter(node);
  formatTypeHead(f);
  auto open = f.keyword("{");
  auto close = f.keyword("}");
  open.prepend(newLine).append(newLine({.allowMore = true}));
  f.interior(open, close).prepend(indent({.allowMore = true}));
  f.keywords(",").prepend(noSpace).append(newLine);
  close.prepend(newLine).append(newLine({.allowMore = true}));
}

void XsmpcatFormatter::formatCollectionLiteral(
    Builder& builder, const CollectionLiteral* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("{").surround(oneSpace);
  f.keywords(",").prepend(noSpace).append(oneSpace);
  f.keyword("}").prepend(oneSpace);
}

void XsmpcatFormatter::formatDesignatedInitializer(
    Builder& builder, const DesignatedInitializer* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword(".").append(noSpace);
  f.keyword("=").surround(oneSpace);
}

void XsmpcatFormatter::formatUnaryOperation(Builder& builder,
                                            const UnaryOperation* node) const {
  // The operator text is not a plain assignment, so it has no property to
  // select: select it by its literal text.
  builder.getNodeFormatter(node).keyword(node->feature).append(noSpace);
}

void XsmpcatFormatter::formatBinaryOperation(
    Builder& builder, const BinaryOperation* node) const {
  builder.getNodeFormatter(node).keyword(node->feature).surround(oneSpace);
}

void XsmpcatFormatter::formatParenthesizedExpression(
    Builder& builder, const ParenthesizedExpression* node) const {
  builder.getNodeFormatter(node)
      .property<&ParenthesizedExpression::expr>()
      .surround(noSpace);
}

template <class T>
void XsmpcatFormatter::formatBuiltIn(Builder& builder, const T* node) const {
  const auto f = builder.getNodeFormatter(node);
  f.keyword("$").append(noSpace);
  f.keyword("(").surround(noSpace);
  f.keyword(")").prepend(noSpace);
}

XsmpcatFormatter::XsmpcatFormatter(const cursiv::CoreServices& services)
    : AbstractFormatter(services) {
  using Self = XsmpcatFormatter;
  on<Attribute>(&Self::formatAttribute);
  on<Parameter>(&Self::formatParameter);
  on<ReturnParameter>(&Self::formatReturnParameter);
  on<Container>(&Self::formatMultipleMember<Container, &Container::type>);
  on<Reference>(&Self::formatMultipleMember<Reference, &Reference::interface_>);
  on<Multiplicity>(&Self::formatMultiplicity);
  on<EntryPoint>(&Self::formatEntryPoint);
  on<EventSource>(&Self::formatEventMember<EventSource>);
  on<EventSink>(&Self::formatEventMember<EventSink>);
  on<EnumerationLiteral>(&Self::formatEnumerationLiteral);
  on<Catalogue>(&Self::formatCatalogue);
  on<Namespace>(&Self::formatNamespace);
  on<Constant>(&Self::formatTypedMember<Constant>);
  on<Field>(&Self::formatTypedMember<Field>);
  on<Association>(&Self::formatTypedMember<Association>);
  on<AttributeType>(&Self::formatTypedMember<AttributeType>);
  on<Property>(&Self::formatProperty);
  on<Operation>(&Self::formatOperation);
  on<Class>(&Self::formatBlockType<Class>);
  on<Exception>(&Self::formatBlockType<Exception>);
  on<Structure>(&Self::formatBlockType<Structure>);
  on<Model>(&Self::formatBlockType<Model>);
  on<Service>(&Self::formatBlockType<Service>);
  on<Interface>(&Self::formatBlockType<Interface>);
  on<Integer>(&Self::formatInteger);
  on<Float>(&Self::formatFloat);
  on<Enumeration>(&Self::formatEnumeration);
  on<Array>(&Self::formatLineType<Array>);
  on<String>(&Self::formatLineType<String>);
  on<ValueReference>(&Self::formatLineType<ValueReference>);
  on<EventType>(&Self::formatLineType<EventType>);
  on<NativeType>(&Self::formatLineType<NativeType>);
  on<PrimitiveType>(&Self::formatLineType<PrimitiveType>);
  on<CollectionLiteral>(&Self::formatCollectionLiteral);
  on<DesignatedInitializer>(&Self::formatDesignatedInitializer);
  on<UnaryOperation>(&Self::formatUnaryOperation);
  on<BinaryOperation>(&Self::formatBinaryOperation);
  on<ParenthesizedExpression>(&Self::formatParenthesizedExpression);
  on<BuiltInConstant>(&Self::formatBuiltIn<BuiltInConstant>);
  on<BuiltInFunction>(&Self::formatBuiltIn<BuiltInFunction>);
}

}  // namespace xsmp
