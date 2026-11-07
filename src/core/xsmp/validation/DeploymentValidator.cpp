#include <algorithm>
#include <cstdint>
#include <cursiv/workspace/Document.hpp>
#include <span>
#include <string>
#include <utility>
#include <vector>
#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/solver/Solver.hpp>
#include <xsmp/utils/TreeUtils.hpp>
#include <xsmp/utils/XsmpUtils.hpp>
#include <xsmp/validation/DeploymentValidator.hpp>
#include <xsmp/validation/XsmpDocumentValidator.hpp>

namespace xsmp {

bool checkPattern(std::string_view name, const AstNode& node,
                  std::string_view context,
                  cursiv::ValidationAcceptor& accept) {
  if (name.find('{') == std::string_view::npos) return true;
  const auto bindings = bindingsFor(&node);
  const auto knownParameters = templateParameterNames(node);
  auto valid = true;
  for (auto at = name.find('{'); at != std::string_view::npos;
       at = name.find('{', at + 1)) {
    const auto end = name.find('}', at + 1);
    if (end == std::string_view::npos) break;
    const auto parameter = name.substr(at + 1, end - at - 1);
    if (!knownParameters.contains(parameter)) {
      accept
          .error(node, "The placeholder '{" + std::string(parameter)
                           + "}' shall resolve to a Template Argument of the "
                           + std::string(context) + ".")
          .range(rangeForFeature(
              node, cursiv::ast_cast<ModelInstance>(&node)
                            || cursiv::ast_cast<AssemblyInstance>(&node)
                            || cursiv::ast_cast<Task>(&node)
                        ? "name"
                        : ""));
      valid = false;
    }
  }

  // `expanded` gives nothing while a placeholder has no value.
  if (const auto concrete = expanded(name, bindings);
      concrete && !validIdentifier(*concrete)) {
    const auto instanceName = cursiv::ast_cast<ModelInstance>(&node)
                              || cursiv::ast_cast<AssemblyInstance>(&node)
                              || cursiv::ast_cast<Task>(&node);
    accept
        .error(node, std::string(instanceName ? "The expanded name '"
                                              : "The expanded path segment '")
                         + *concrete + "' is not valid for SMP Level 2.")
        .range(rangeForFeature(node, instanceName ? "name" : ""));
    valid = false;
  } else if (concrete && isCppKeyword(*concrete)
             && (cursiv::ast_cast<ModelInstance>(&node)
                 || cursiv::ast_cast<AssemblyInstance>(&node))) {
    accept
        .warning(node, "The expanded name '" + *concrete
                           + "' shall not be an ISO/ANSI C++ keyword.")
        .range(rangeForFeature(node, "name"));
  }
  return valid;
}

void DeploymentValidator::checkDocument(const Document& node,
                                        cursiv::ValidationAcceptor& accept) {
  checkName(node, accept);
  checkEncoding(node, accept);
  checkEscapes(node, accept);
  checkOutputFile(node, accept);

  std::vector<cursiv::DiagnosticRelatedInformation> related;
  const auto name = nameOf(&node);
  const auto& language = *node.getDocument()->language;
  (void)language.shared().indexManager->visitByName(
      name, true, {}, *language.astReflection, [&](const auto& d) {
        if (d.type && d.type->derivesFrom(&cursiv::ast_type_tag<Document>)
            && d.node != &node && d.node)
          related.push_back({.uri = d.node->getDocument()->uri,
                             .message = std::string(d.name),
                             .range = rangeForFeature(*d.node, "name")});
        return true;
      });

  if (!related.empty())
    accept.error(node, "Duplicated Document name.")
        .range(rangeForFeature(node, "name"))
        .relatedInformation(
            std::span<const cursiv::DiagnosticRelatedInformation>(related));
}

void DeploymentValidator::checkPath(const Path& node,
                                    cursiv::ValidationAcceptor& accept) const {
  const auto* field = cursiv::ast_cast<FieldValue>(node.parent);
  if (!field || !cursiv::ast_cast<StructureValue>(field->parent))
    xsmp::checkPath(&node, accept);
}

void DeploymentValidator::checkFieldValue(
    const FieldValue& node, cursiv::ValidationAcceptor& accept) const {
  if (node.field && !node.field->unsafe
      && !cursiv::ast_cast<StructureValue>(node.parent))
    checkValue(node.value,
               cursiv::ast_cast<Type>(targetType(pathTarget(node.field))),
               accept);
}

namespace {

bool fitsInt32(std::int64_t value) {
  return std::in_range<std::int32_t>(value);
}

// Whether a path gives its value no type: it is unsafe, or nothing says what it
// reads in. A context that does not resolve is reported on its own.
bool typesNothing(const Path* path) {
  if (!path || path->unsafe) return true;
  if (pathContext(path)) return false;
  if (const auto* configuration =
          cursiv::container_of_type<ComponentConfiguration>(path))
    return configuration->context.refText.empty();
  if (const auto* task = cursiv::container_of_type<Task>(path))
    return task->context.refText.empty();
  return true;
}

// A path that does not resolve is reported on its own, so a value behind it
// says nothing more.
bool hasNoKnownType(const Value& value) {
  for (const auto* at = value.parent; at; at = at->parent) {
    if (const auto* field = cursiv::ast_cast<FieldValue>(at);
        field && field->field)
      return typesNothing(field->field);
    if (const auto* field = cursiv::ast_cast<CfgStructureFieldValue>(at))
      return !field->field || field->field->unsafe;
    if (const auto* property = cursiv::ast_cast<SetProperty>(at))
      return typesNothing(property->propertyPath);
    if (const auto* property = cursiv::ast_cast<PropertyValue>(at))
      return !property->property || property->property->unsafe;
    if (const auto* call = cursiv::ast_cast<CallOperation>(at))
      return typesNothing(call->operationPath);
    if (const auto* call = cursiv::ast_cast<OperationCall>(at))
      return !call->operation || call->operation->unsafe;
  }
  return true;
}

bool inStandard2025(const cursiv::AstNode& node) {
  const auto* document = node.getDocument();
  const auto* shared = document ? dynamic_cast<const XsmpSharedCoreServices*>(
                                      &document->language->shared())
                                : nullptr;
  const auto* project = shared && shared->projectManager
                            ? shared->projectManager->getProject(*document)
                            : nullptr;
  return project && project->standard == "ECSS_SMP_2025";
}

// Whether SMP XML can give a start index to this array: its type is a simple
// array or, when nothing gives it a type, its items are all of one simple kind
// that each one spells out. A path that does not resolve is reported on its
// own.
bool canHaveStartIndex(const ArrayValue& node) {
  if (const auto* array =
          cursiv::ast_cast<Array>(inferDeploymentValueType(&node)))
    return getPTK(array->itemType.get()) != PTK::None
           && isSimpleArrayAttribute(*array);
  if (!hasNoKnownType(node)) return true;
  const auto spelled = [](const Value* item) {
    return cursiv::ast_cast<SimpleValue>(item)
           && !cursiv::ast_cast<IntValue>(item)
           && !cursiv::ast_cast<FloatValue>(item);
  };
  if (node.elements.empty() || !std::ranges::all_of(node.elements, spelled))
    return false;
  const auto* first = node.elements[0]->type;
  return std::ranges::all_of(
      node.elements, [&](const Value* item) { return item->type == first; });
}

// `Assembly:Name` of the SMP schema, `(([a-zA-Z0-9_])|\{[a-zA-Z0-9_]*\})*`.
bool isAssemblyName(std::string_view text) {
  const auto word = [](unsigned char c) {
    return c == '_' || (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z')
           || (c >= 'A' && c <= 'Z');
  };
  while (!text.empty()) {
    if (text.front() != '{') {
      if (!word(text.front())) return false;
      text.remove_prefix(1);
      continue;
    }
    const auto end = text.find('}');
    if (end == text.npos || !std::ranges::all_of(text.substr(1, end - 1), word))
      return false;
    text.remove_prefix(end + 1);
  }
  return true;
}

constexpr const char* notAssemblyName =
    "A template value shall hold only letters, digits, '_' and '{parameter}'.";

}  // namespace

void DeploymentValidator::checkIntValue(
    const IntValue& node, cursiv::ValidationAcceptor& accept) const {
  if (hasNoKnownType(node)
      && !cursiv::ast_cast<Type>(inferDeploymentValueType(&node)))
    accept.error(node,
                 "This value has no known type: write a suffix, as in 1i32.");
}

void DeploymentValidator::checkFloatValue(
    const FloatValue& node, cursiv::ValidationAcceptor& accept) const {
  if (hasNoKnownType(node)
      && !cursiv::ast_cast<Type>(inferDeploymentValueType(&node)))
    accept.error(node,
                 "This value has no known type: write a suffix, as in 1.0f64.");
}

void DeploymentValidator::checkArrayValue(
    const ArrayValue& node, cursiv::ValidationAcceptor& accept) const {
  if (!node.startIndex) return;
  if (*node.startIndex < 0)
    accept.error(node, "StartIndex shall not be negative.");
  const auto refuse = [&](const char* message) {
    accept.error(node, message).property<&ArrayValue::startIndex>();
  };
  if (!inStandard2025(node))
    refuse("StartIndex is only available in ECSS_SMP_2025.");
  if (!cursiv::container_of_type<Configuration>(&node))
    refuse("StartIndex is only available in a Configuration.");
  else if (!canHaveStartIndex(node))
    refuse("StartIndex is only allowed for SimpleArray values.");
}

void DeploymentValidator::checkChar8Value(
    const Char8Value& node, cursiv::ValidationAcceptor& accept) const {
  if (node.value.size() != 1 && !holdsReportedLiteral(node))
    accept.error(node, "A Char8 value shall contain exactly one character.");
}

void DeploymentValidator::checkInt32Parameter(
    const Int32Parameter& node, cursiv::ValidationAcceptor& accept) const {
  checkName(node, accept);
  if (!node.value)
    accept.error(node, "A template parameter needs a value.");
  else if (!fitsInt32(*node.value))
    accept.error(node, "Conversion overflow for type Int32.");
}

void DeploymentValidator::checkStringParameter(
    const StringParameter& node, cursiv::ValidationAcceptor& accept) const {
  checkName(node, accept);
  if (holdsReportedLiteral(node)) return;
  // Only an absent value has no data: an empty string still points into the
  // text.
  if (!node.value.data())
    accept.error(node, "A template parameter needs a value.");
  else if (!isAssemblyName(node.value))
    accept.error(node, notAssemblyName).property<&StringParameter::value>();
}

void DeploymentValidator::checkTemplateArguments(
    const std::vector<TemplateArgument*>& arguments,
    cursiv::ValidationAcceptor& accept) {
  std::unordered_set<std::string_view> names;
  for (const auto* argument : arguments) {
    if (!names.insert(argument->parameter.refText).second)
      accept.error(*argument, "Duplicated template argument name.");
    const auto* parameter = argument->parameter.get();
    const auto* number = cursiv::ast_cast<Int32Argument>(argument);
    if (number && !fitsInt32(number->value))
      accept.error(*argument, "Conversion overflow for type Int32.");
    if (parameter
        && (number ? !cursiv::ast_cast<Int32Parameter>(parameter)
                   : !cursiv::ast_cast<StringParameter>(parameter)))
      accept.error(*argument, "The value shall have the type of the parameter "
                                  + std::string(parameter->name) + ".");
    if (const auto* text = cursiv::ast_cast<StringArgument>(argument);
        text && !isAssemblyName(text->value) && !holdsReportedLiteral(*text))
      accept.error(*text, notAssemblyName).property<&StringArgument::value>();
  }
}

void DeploymentValidator::checkName(const cursiv::AstNode& node,
                                    cursiv::ValidationAcceptor& accept) {
  const auto name = nameOf(&node);
  if (name.empty())
    accept.error(node, "A name shall not be empty.")
        .range(rangeForFeature(node, "name"));
  else if (name.find('{') == std::string_view::npos && !validIdentifier(name))
    accept.error(node, "A name shall start with a letter.")
        .range(rangeForFeature(node, "name"));
}

void DeploymentValidator::checkInstanceName(
    const cursiv::AstNode& node, std::string_view owner,
    cursiv::ValidationAcceptor& accept) {
  checkName(node, accept);
  checkPattern(nameOf(&node), node, owner, accept);
}

std::unordered_set<std::string_view> DeploymentValidator::usedParameters(
    const Document& root) {
  std::unordered_set<std::string_view> used;
  for (const auto* node : descendantsOf(root)) {
    std::string_view text;
    if (const auto* segment = cursiv::ast_cast<PatternPathNamedSegment>(node))
      text = segment->text;
    else if (cursiv::ast_cast<ModelInstance>(node)
             || cursiv::ast_cast<AssemblyInstance>(node)
             || cursiv::ast_cast<Task>(node))
      text = nameOf(node);
    for (auto at = text.find('{'); at != std::string_view::npos;
         at = text.find('{', at + 1)) {
      const auto end = text.find('}', at + 1);
      if (end != std::string_view::npos)
        used.insert(text.substr(at + 1, end - at - 1));
    }
  }
  return used;
}

void DeploymentValidator::checkUnusedParameters(
    const std::vector<TemplateParameter*>& parameters,
    const std::unordered_set<std::string_view>& used,
    cursiv::ValidationAcceptor& accept) {
  for (auto* parameter : parameters)
    if (!parameter->name.empty() && !used.contains(parameter->name))
      accept
          .warning(*parameter, "The Template Parameter '"
                                   + std::string(parameter->name)
                                   + "' is not used.")
          .property<&TemplateParameter::name>();
}

void DeploymentValidator::checkDuplicatedParameters(
    const std::vector<TemplateParameter*>& parameters,
    cursiv::ValidationAcceptor& accept) {
  std::unordered_set<std::string_view> names;
  for (auto* parameter : parameters)
    if (parameter && !names.insert(parameter->name).second)
      accept.error(*parameter, "Duplicated template argument name.")
          .range(rangeForFeature(*parameter, "name"));
}

void DeploymentValidator::checkDuplicatedCallParameters(
    const std::vector<ParameterValue*>& parameters,
    cursiv::ValidationAcceptor& accept) {
  std::unordered_set<std::string_view> names;
  for (auto* parameter : parameters)
    if (parameter && !names.insert(parameter->parameter).second)
      accept.error(*parameter, "Duplicated parameter name.");
}

void DeploymentValidator::checkPropertyAssignment(
    const cursiv::AstNode& source, std::string_view feature,
    const Property* property, const Value* value,
    cursiv::ValidationAcceptor& accept) {
  if (!property) return;
  if (hasModifier(*property, "readOnly"))
    accept.error(source, "A PropertyValue shall target a writable Property.")
        .range(rangeForFeature(source, feature));
  checkValue(value, property->type.get(), accept);
}

}  // namespace xsmp
