#pragma once

#include <cursiv/validation/ValidationAcceptor.hpp>
#include <cursiv/validation/ValidationRegistry.hpp>
#include <string_view>
#include <unordered_set>
#include <xsmp/ast.hpp>
#include <xsmp/utils/XsmpUtils.hpp>

namespace xsmp {

/// The checks of the rules the deployment languages share. A language registers
/// those of the rules its grammar reads, and adds the checks of its own rules.
class DeploymentValidator {
 public:
  // The path of a field value inside a structure value is left to `checkValue`,
  // which reads it against the type of that structure.
  void checkPath(const Path& node, cursiv::ValidationAcceptor& accept) const;

  // The value against the type of the field its path names. Field values inside
  // a structure value are checked with that structure.
  void checkFieldValue(const FieldValue& node,
                       cursiv::ValidationAcceptor& accept) const;

  // A number without a type suffix takes its type from where it is written.
  // This and `checkFloatValue` report one where nothing gives it a type.
  void checkIntValue(const IntValue& node,
                     cursiv::ValidationAcceptor& accept) const;

  void checkFloatValue(const FloatValue& node,
                       cursiv::ValidationAcceptor& accept) const;

  // A StartIndex stands only where SMP XML has one: on a simple array of a
  // Configuration of ECSS SMP 2025.
  void checkArrayValue(const ArrayValue& node,
                       cursiv::ValidationAcceptor& accept) const;

  void checkChar8Value(const Char8Value& node,
                       cursiv::ValidationAcceptor& accept) const;

  void checkInt32Parameter(const Int32Parameter& node,
                           cursiv::ValidationAcceptor& accept) const;

  void checkStringParameter(const StringParameter& node,
                            cursiv::ValidationAcceptor& accept) const;

 protected:
  // The name of a document, unique among the documents of the workspace, and
  // the file it writes, unique in its project.
  static void checkDocument(const Document& node,
                            cursiv::ValidationAcceptor& accept);

  // Not empty, and a valid identifier. A name with a placeholder is left to
  // `checkPattern`, which checks its expansion.
  static void checkName(const cursiv::AstNode& node,
                        cursiv::ValidationAcceptor& accept);

  // The name of an instance or a task, which may hold placeholders resolved by
  // the template parameters of `owner`.
  static void checkInstanceName(const cursiv::AstNode& node,
                                std::string_view owner,
                                cursiv::ValidationAcceptor& accept);

  // The placeholders written under `root`, in path segments and in the names of
  // instances and tasks.
  [[nodiscard]] static std::unordered_set<std::string_view> usedParameters(
      const Document& root);

  static void checkUnusedParameters(
      const std::vector<TemplateParameter*>& parameters,
      const std::unordered_set<std::string_view>& used,
      cursiv::ValidationAcceptor& accept);

  static void checkDuplicatedParameters(
      const std::vector<TemplateParameter*>& parameters,
      cursiv::ValidationAcceptor& accept);

  static void checkDuplicatedCallParameters(
      const std::vector<ParameterValue*>& parameters,
      cursiv::ValidationAcceptor& accept);

  // Each argument named once, with a value of the type of its parameter.
  static void checkTemplateArguments(
      const std::vector<TemplateArgument*>& arguments,
      cursiv::ValidationAcceptor& accept);

  // A value set on a Property: it shall be writable, and the value fit its
  // type. `feature` of `source` is where a read-only Property is reported. A
  // null Property checks nothing.
  static void checkPropertyAssignment(const cursiv::AstNode& source,
                                      std::string_view feature,
                                      const Property* property,
                                      const Value* value,
                                      cursiv::ValidationAcceptor& accept);
};

}  // namespace xsmp
