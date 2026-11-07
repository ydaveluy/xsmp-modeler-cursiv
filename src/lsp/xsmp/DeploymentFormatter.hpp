#pragma once

#include <cursiv/formatting/AbstractFormatter.hpp>
#include <xsmp/ast.hpp>

namespace xsmp {

/// The rules of what the deployment languages share. Each language adds the
/// rules of its own constructions.
class DeploymentFormatter : public cursiv::AbstractFormatter {
 public:
  explicit DeploymentFormatter(const cursiv::CoreServices& services);

 protected:
  template <class Rules>
  static void formatCommas(const Rules& f) {
    f.keywords(",").prepend(noSpace).append(oneSpace);
  }

  // `{` and `}` each on its own line, with the content between them indented.
  template <class Rules>
  static void formatBody(const Rules& f) {
    auto open = f.keyword("{");
    auto close = f.keyword("}");
    open.prepend(newLine).append(newLine({.allowMore = true}));
    f.interior(open, close).prepend(indent);
    close.prepend(newLine).append(newLine({.allowMore = true}));
  }

  template <class Rules>
  static void formatAssignment(const Rules& f) {
    f.keyword("=").surround(oneSpace);
  }

  // `name: Type`.
  template <class Rules>
  static void formatAnnotation(const Rules& f) {
    f.keyword(":").prepend(noSpace).append(oneSpace);
  }

  // `<a, b>`. `separated` puts one space before it, otherwise none.
  template <class Rules>
  static void formatAngles(const Rules& f, bool separated) {
    f.keyword("<")
        .prepend(separated ? static_cast<cursiv::FormattingAction>(oneSpace)
                           : static_cast<cursiv::FormattingAction>(noSpace))
        .append(noSpace);
    formatCommas(f);
    f.keyword(">").prepend(noSpace);
  }

  // `call <operation>(<parameters>)`.
  template <class Rules>
  static void formatCall(const Rules& f) {
    f.keyword("call").append(oneSpace);
    f.keyword("(").surround(noSpace);
    formatCommas(f);
    f.keyword(")").prepend(noSpace);
  }

  // `property <property> = <value>`.
  template <class Rules>
  static void formatProperty(const Rules& f) {
    f.keyword("property").append(oneSpace);
    formatAssignment(f);
  }

 private:
  template <class T>
  void formatUnsafe(cursiv::FormattingBuilder& builder, const T* node) const;
  template <class T>
  void formatParameter(cursiv::FormattingBuilder& builder, const T* node) const;
  template <class T>
  void formatAssigned(cursiv::FormattingBuilder& builder, const T* node) const;
  template <class T>
  void formatLink(cursiv::FormattingBuilder& builder, const T* node) const;
  void formatPath(cursiv::FormattingBuilder& builder, const Path* node) const;
  void formatPathMember(cursiv::FormattingBuilder& builder,
                        const PathMember* node) const;

  void formatPathIndex(cursiv::FormattingBuilder& builder,
                       const PathIndex* node) const;

  void formatArrayValue(cursiv::FormattingBuilder& builder,
                        const ArrayValue* node) const;

  void formatStructureValue(cursiv::FormattingBuilder& builder,
                            const StructureValue* node) const;
};

}  // namespace xsmp
