#pragma once

#include <cursiv/formatting/AbstractFormatter.hpp>
#include <xsmp/ast.hpp>

namespace xsmp {

/// Formatter for xsmpcat.
class XsmpcatFormatter final : public cursiv::AbstractFormatter {
 public:
  explicit XsmpcatFormatter(const cursiv::CoreServices& services);

 private:
  using Builder = cursiv::FormattingBuilder;

  void formatAttribute(Builder&, const Attribute*) const;
  void formatParameter(Builder&, const Parameter*) const;
  void formatReturnParameter(Builder&, const ReturnParameter*) const;
  void formatMultiplicity(Builder&, const Multiplicity*) const;
  void formatEntryPoint(Builder&, const EntryPoint*) const;
  void formatEnumerationLiteral(Builder&, const EnumerationLiteral*) const;
  void formatCatalogue(Builder&, const Catalogue*) const;
  void formatNamespace(Builder&, const Namespace*) const;
  void formatProperty(Builder&, const Property*) const;
  void formatOperation(Builder&, const Operation*) const;
  void formatInteger(Builder&, const Integer*) const;
  void formatFloat(Builder&, const Float*) const;
  void formatEnumeration(Builder&, const Enumeration*) const;
  void formatCollectionLiteral(Builder&, const CollectionLiteral*) const;
  void formatDesignatedInitializer(Builder&,
                                   const DesignatedInitializer*) const;
  void formatUnaryOperation(Builder&, const UnaryOperation*) const;
  void formatBinaryOperation(Builder&, const BinaryOperation*) const;
  void formatParenthesizedExpression(Builder&,
                                     const ParenthesizedExpression*) const;

  // `$PI` and `$sin(x)`.
  template <class T>
  void formatBuiltIn(Builder&, const T*) const;

  // A container or a reference: its type, then its multiplicity with no space.
  template <class T, auto TypeMember>
  void formatMultipleMember(Builder&, const T*) const;

  // An event source or sink.
  template <class T>
  void formatEventMember(Builder&, const T*) const;

  // A member with modifiers, a type, a name and maybe `= value`.
  template <class T>
  void formatTypedMember(Builder&, const T*) const;

  // A type with a body: `{` and `}` each on their own line.
  template <class T>
  void formatBlockType(Builder&, const T*) const;

  // A type written on one line, such as `array A = B[1]` or `native N`.
  template <class T>
  void formatLineType(Builder&, const T*) const;

  // What starts each type: its modifiers, its name and `extends`.
  template <class Rules>
  static void formatTypeHead(const Rules&);

  // Allman-style block: `{`/`}` each on their own line, indented interior.
  // `newLine(allowMore)` keeps (but does not require) existing blank lines.
  template <class Rules>
  static void formatBody(const Rules&);
};

}  // namespace xsmp
