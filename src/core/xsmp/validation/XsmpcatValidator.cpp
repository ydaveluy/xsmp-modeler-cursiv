#include <algorithm>
#include <cstddef>
#include <cursiv/services/Diagnostic.hpp>
#include <cursiv/syntax-tree/AstNode.hpp>
#include <cursiv/workspace/Document.hpp>
#include <cursiv/workspace/Documents.hpp>
#include <cursiv/workspace/IndexManager.hpp>
#include <cursiv/workspace/ReferenceDescriptionProvider.hpp>
#include <functional>
#include <initializer_list>
#include <map>
#include <mutex>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <xsmp/CoreServices.hpp>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/solver/Solver.hpp>
#include <xsmp/utils/AttributeUsage.hpp>
#include <xsmp/utils/TreeUtils.hpp>
#include <xsmp/utils/Utf8.hpp>
#include <xsmp/utils/XsmpUtils.hpp>
#include <xsmp/validation/IssueCodes.hpp>
#include <xsmp/validation/XsmpDocumentValidator.hpp>
#include <xsmp/validation/XsmpcatValidator.hpp>
#include <xsmp/workspace/ProjectManager.hpp>
#include <xsmp/workspace/SourceUri.hpp>

namespace xsmp {

using Acceptor = cursiv::ValidationAcceptor;
using ModifierPredicate = bool (*)(std::string_view);

namespace {

// A name starts with a letter, then holds only word characters.
bool isValidNameStart(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isValidNameChar(char c) {
  return isValidNameStart(c) || (c >= '0' && c <= '9') || c == '_';
}

bool matchesNamePattern(std::string_view name) {
  if (name.empty() || !isValidNameStart(name.front())) {
    return false;
  }
  return std::all_of(name.begin() + 1, name.end(), isValidNameChar);
}

// The recognized `@usage` category names an AttributeType's own `@usage` tags
// may reference (checkAttributeType's declaration-side validation) and that
// `matchesUsageCategory` (checkAttributeUsages' instance-side validation)
// dispatches on.
const std::unordered_set<std::string_view>& validUsageCategories() {
  static const std::unordered_set<std::string_view> names{"NamedElement",
                                                          "Array",
                                                          "Association",
                                                          "AttributeType",
                                                          "Catalogue",
                                                          "Class",
                                                          "Component",
                                                          "Constant",
                                                          "Container",
                                                          "Document",
                                                          "EntryPoint",
                                                          "Enumeration",
                                                          "EnumerationLiteral",
                                                          "EventSink",
                                                          "EventSource",
                                                          "EventType",
                                                          "Exception",
                                                          "Field",
                                                          "Float",
                                                          "Integer",
                                                          "Interface",
                                                          "LanguageType",
                                                          "Model",
                                                          "Namespace",
                                                          "NativeType",
                                                          "Operation",
                                                          "Parameter",
                                                          "PrimitiveType",
                                                          "Property",
                                                          "ReferenceType",
                                                          "Reference",
                                                          "Service",
                                                          "SimpleType",
                                                          "String",
                                                          "Structure",
                                                          "Type",
                                                          "ValueReference",
                                                          "ValueType",
                                                          "VisibilityElement"};
  return names;
}

// The kinds an Integer type may store. Unlike `isIntegralType`, this leaves out
// DateTime and Duration.
bool isIntegerKind(PTK kind) {
  switch (kind) {
    case PTK::Int8:
    case PTK::Int16:
    case PTK::Int32:
    case PTK::Int64:
    case PTK::UInt8:
    case PTK::UInt16:
    case PTK::UInt32:
    case PTK::UInt64:
      return true;
    default:
      return false;
  }
}

// Reports the modifiers that repeat or contradict an earlier one, and those not
// allowed on `element`, given the predicates that classify the allowed ones.
void checkModifiers(const VisibilityElement& element,
                    std::initializer_list<ModifierPredicate> predicates,
                    Acceptor& accept) {
  for (const auto predicate : predicates) {
    std::optional<std::string_view> first;
    for (std::size_t i = 0; i < element.modifiers.size(); ++i) {
      const auto modifier = element.modifiers[i];
      if (!predicate(modifier)) continue;
      if (first)
        accept
            .error(element,
                   modifier == *first
                       ? "Duplicate modifier '" + std::string(modifier) + "'."
                       : "'" + std::string(modifier) + "' conflicts with '"
                             + std::string(*first) + "'.")
            .property<&VisibilityElement::modifiers>(i)
            .data(diagnosticData(IllegalModifier));
      else
        first = modifier;
    }
  }

  for (std::size_t i = 0; i < element.modifiers.size(); ++i)
    if (std::ranges::none_of(predicates, [&](const auto predicate) {
          return predicate(element.modifiers[i]);
        }))
      accept
          .error(element, "The modifier '" + std::string(element.modifiers[i])
                              + "' is not allowed here.")
          .property<&VisibilityElement::modifiers>(i)
          .data(diagnosticData(InvalidModifier));
}

using DuplicateMemberMap =
    std::unordered_map<std::string_view, std::vector<const NamedElement*>>;

// Recursively collects the members of `type`'s base(s)/interfaces into
// `duplicates` (never `type`'s OWN elements — the caller seeds those
// incrementally).
void collectInheritedMembers(const Type* type, DuplicateMemberMap& duplicates,
                             std::unordered_set<const Type*>& visited) {
  if (type == nullptr || !visited.insert(type).second) {
    return;
  }
  if (const auto* interface = cursiv::ast_cast<const Interface>(type)) {
    for (const auto* element : interface->elements) {
      if (element != nullptr) {
        duplicates[element->name].push_back(element);
      }
    }
    for (const auto& base : interface->base) {
      collectInheritedMembers(base.get(), duplicates, visited);
    }
  } else if (const auto* component = cursiv::ast_cast<const Component>(type)) {
    for (const auto* element : component->elements) {
      if (element != nullptr) {
        duplicates[element->name].push_back(element);
      }
    }
    collectInheritedMembers(
        component->base.has_value() ? component->base->get() : nullptr,
        duplicates, visited);
    for (const auto& interfaceRef : component->interface_) {
      collectInheritedMembers(interfaceRef.get(), duplicates, visited);
    }
  } else if (const auto* structure = cursiv::ast_cast<const Structure>(type)) {
    for (const auto* element : structure->elements) {
      if (element != nullptr) {
        duplicates[element->name].push_back(element);
      }
    }
    if (const auto* clazz = cursiv::ast_cast<const Class>(type)) {
      collectInheritedMembers(
          clazz->base.has_value() ? clazz->base->get() : nullptr, duplicates,
          visited);
    }
  }
}

// Seeds a duplicate-name map with `type`'s INHERITED members only.
DuplicateMemberMap initDuplicatedMembers(const Type& type) {
  DuplicateMemberMap duplicates;
  std::unordered_set<const Type*> visited{&type};
  if (const auto* interface = cursiv::ast_cast<const Interface>(&type)) {
    for (const auto& base : interface->base) {
      collectInheritedMembers(base.get(), duplicates, visited);
    }
  } else if (const auto* component = cursiv::ast_cast<const Component>(&type)) {
    collectInheritedMembers(
        component->base.has_value() ? component->base->get() : nullptr,
        duplicates, visited);
    for (const auto& interfaceRef : component->interface_) {
      collectInheritedMembers(interfaceRef.get(), duplicates, visited);
    }
  } else if (const auto* clazz = cursiv::ast_cast<const Class>(&type)) {
    collectInheritedMembers(
        clazz->base.has_value() ? clazz->base->get() : nullptr, duplicates,
        visited);
  }
  return duplicates;
}

// Flags `member` when its name collides with an already-seen member (either
// inherited or an earlier own sibling), then records it so later members see it
// too. an Operation only collides with a non-Operation, or with a
// same-signature Operation declared in the SAME container (an exact
// redeclaration — overriding a base's operation with the same signature is
// allowed); a Constant/Property only collides with another member declared in
// the SAME container (shadowing an inherited Constant/Property is allowed);
// anything else (Field/Association/...) collides on any name match, inherited
// or not.
void checkDuplicatedMember(DuplicateMemberMap& duplicates,
                           const NamedElement& member, Acceptor& accept) {
  if (member.name.empty()) {
    return;
  }
  auto& bucket = duplicates[member.name];
  if (!bucket.empty()) {
    auto flagged = false;
    if (cursiv::ast_cast<const Operation>(&member) != nullptr) {
      flagged = std::ranges::any_of(bucket, [&](const NamedElement* duplicate) {
        if (cursiv::ast_cast<const Operation>(duplicate) == nullptr) {
          return true;
        }
        return member.parent == duplicate->parent
               && getSignature(member) == getSignature(*duplicate);
      });
    } else if (cursiv::ast_cast<const Constant>(&member) != nullptr
               || cursiv::ast_cast<const Property>(&member) != nullptr) {
      flagged = std::ranges::any_of(bucket, [&](const NamedElement* duplicate) {
        return duplicate->parent == member.parent;
      });
    } else {
      flagged = true;
    }

    if (flagged) {
      std::vector<cursiv::DiagnosticRelatedInformation> related;
      const auto* operation = cursiv::ast_cast<const Operation>(&member);
      const auto signature = operation ? getSignature(member) : std::string{};
      // List only the real collisions: an overload in the same bucket is not
      // one.
      for (const auto* duplicate : bucket) {
        if (operation && cursiv::ast_cast<const Operation>(duplicate)
            && (signature != getSignature(*duplicate)
                || member.parent != duplicate->parent))
          continue;
        const auto* document = duplicate->getDocument();
        if (!document || duplicate->cstNodeId == cursiv::kNoNode) continue;
        const auto& cst = document->parseResult.cst()[duplicate->cstNodeId];
        related.push_back({.uri = std::string(document->uri),
                           .message = qualifiedName(*duplicate),
                           .range = {.begin = cst.begin, .end = cst.end}});
      }

      accept
          .error(member, "Duplicated name '" + std::string(member.name) + "'.")
          .property<&NamedElement::name>()
          .relatedInformation(related);
    }
  }
  bucket.push_back(&member);
}

// Seeds from `type`'s inherited members, then checks + accumulates each of
// `type`'s own `elements` in declaration order.
void checkDuplicatedMembers(const Type& type,
                            const std::vector<NamedElement*>& members,
                            Acceptor& accept) {
  auto duplicates = initDuplicatedMembers(type);
  for (const auto* member : members) {
    if (member != nullptr) {
      checkDuplicatedMember(duplicates, *member, accept);
    }
  }
}

// Applies the modifier rules to a member according to its kind and the kind of
// its enclosing classifier.
enum class Owner { Structure, Class, Component, Interface };

void checkMemberModifiers(const NamedElement& member, Owner owner,
                          Acceptor& accept) {
  const auto* visible = cursiv::ast_cast<const VisibilityElement>(&member);
  if (visible == nullptr) {
    return;  // containers/references/entrypoints/events carry no modifiers
  }
  const auto isConstant = cursiv::ast_cast<const Constant>(visible) != nullptr;
  const auto isField = cursiv::ast_cast<const Field>(visible) != nullptr;
  const auto isProperty = cursiv::ast_cast<const Property>(visible) != nullptr;

  // An empty list allows no modifier at all.
  switch (owner) {
    case Owner::Structure:
      if (isConstant) {
        checkModifiers(*visible, {}, accept);
      } else if (isField) {
        checkModifiers(
            *visible,
            {&isInputModifier, &isOutputModifier, &isTransientModifier},
            accept);
      }
      break;
    case Owner::Class:
    case Owner::Component:
      if (isField) {
        checkModifiers(*visible,
                       {&isVisibilityModifier, &isInputModifier,
                        &isOutputModifier, &isTransientModifier},
                       accept);
      } else if (isProperty) {
        checkModifiers(*visible, {&isVisibilityModifier, &isAccessKind},
                       accept);
      } else {
        checkModifiers(*visible, {&isVisibilityModifier}, accept);
      }
      break;
    case Owner::Interface:
      if (isProperty) {
        checkModifiers(*visible, {&isAccessKind}, accept);
      } else {
        checkModifiers(*visible, {}, accept);
      }
      break;
  }
}

// Defined below (after the kind-check helpers); used by
// checkConstant/checkField. For an ArrayType/Structure, recursively validates a
// CollectionLiteral against the item type / non-static-public fields (base
// chain included); for anything else, delegates to `getValueAs(expr, type,
// accept)` for the scalar-level diagnostics (range/length/enum/etc., see
// Solver.{hpp,cpp}).
void checkValueBounds(const Type& type, const Expression* expr,
                      Acceptor& accept);

// `checkValueBounds` for the value of an attribute, which is never written in
// C++ and so can never be a pointer.
void checkAttributeValue(const Type& type, const Expression* expr,
                         Acceptor& accept) {
  if (isNullptr(expr))
    accept.error(*expr, nullptrNeedsAPointer);
  else
    checkValueBounds(type, expr, accept);
}

// True if `elements` contains an Operation or Property whose owner shall
// therefore be abstract.
bool hasAbstractMember(const std::vector<NamedElement*>& elements) {
  return std::ranges::any_of(elements, [&](const auto* element) {
    if (element == nullptr) {
      return false;
    }
    const bool isOperationOrProperty =
        cursiv::ast_cast<const Operation>(element) != nullptr
        || cursiv::ast_cast<const Property>(element) != nullptr;
    return isOperationOrProperty && isAbstractAttribute(*element);
  });
}

// Attaches the visibility error and the deprecated warning of a resolved Field
// reference (`Property.attachedField`, `EntryPoint.input`/`output`): visible
// when `field` is a member of `node`'s own enclosing type, or when `field` is
// not private; plus a `@deprecated` doc-tag warning.
template <auto Property, class Node>
void checkFieldReferenceExtras(const Node& node, const Field& field,
                               Acceptor& accept, std::size_t index = 0) {
  if (node.parent != field.parent && isEffectivelyPrivate(field)) {
    accept.error(node, "The Field is not visible.")
        .template property<Property>(index)
        .data(diagnosticData(FieldNotVisible));
  }

  if (const auto deprecated = docTag(field, "deprecated")) {
    accept
        .warning(node, deprecated->empty()
                           ? "Deprecated."
                           : "Deprecated: " + utf8Text(*deprecated))
        .template property<Property>(index)
        .tags({cursiv::DiagnosticTag::Deprecated});
  }
}

// Attaches the visibility error and the deprecated warning of a resolved type
// reference, once the caller has confirmed that `resolved`'s kind is
// acceptable. Anchored on the `index`-th occurrence of the `Property` feature.
template <auto Property, typename Node>
void checkTypeReferenceExtras(const Node& node, const Type& resolved,
                              Acceptor& accept, std::size_t index = 0) {
  if (!isTypeVisibleFrom(node, resolved)) {
    accept
        .error(node, "The " + std::string(resolved.type->name) + " "
                         + qualifiedName(resolved) + " is not visible.")
        .template property<Property>(index)
        .data(diagnosticData(TypeNotVisible));
  }
  if (const auto deprecated = docTag(resolved, "deprecated")) {
    accept
        .warning(node, deprecated->empty()
                           ? "Deprecated."
                           : "Deprecated: " + utf8Text(*deprecated))
        .template property<Property>(index)
        .tags({cursiv::DiagnosticTag::Deprecated});
  }
}

}  // namespace

XsmpcatValidator::XsmpcatValidator(const XsmpcatCoreServices& services)
    : services(services),
      _workspaceFacts(services.shared),
      _attributeTypeDocs(services.shared),
      _typeGraphs(services.shared) {}

// For each `@Foo(1)` USAGE on `element` (distinct from `checkAttributeType`'s
// DECLARATION-side check), warns when `element`'s own kind doesn't match
// any of the AttributeType's declared `@usage` categories (no categories at all
// means "usable anywhere"), and when a non-`@allowMultiple` AttributeType is
// used more than once on the same element. Templated (rather than fixed to
// `NamedElement`) so it also serves `ReturnParameter`, which carries its own
// `attributes` list but does NOT extend `NamedElement` in this AST.
template <typename T>
void XsmpcatValidator::checkAttributeUsages(const T& element,
                                            Acceptor& accept) const {
  std::unordered_set<const AttributeType*> visited;
  for (const auto* attribute : element.attributes) {
    if (attribute == nullptr) {
      continue;
    }
    const auto* attributeType =
        cursiv::ast_cast<const AttributeType>(attribute->type.get());
    if (attributeType == nullptr) {
      continue;  // The linker reports a reference it could not resolve.
    }

    checkTypeReferenceExtras<&Attribute::type>(*attribute, *attributeType,
                                               accept);
    if (attribute->value) {
      if (const auto* type = attributeType->type.get())
        checkAttributeValue(*type, attribute->value, accept);
    } else if (!attributeType->default_) {
      accept.error(*attribute, "A value is required.")
          .template property<&Attribute::value>()
          .data(diagnosticData(MissingValue));
    }

    const auto doc = attributeTypeDoc(*attributeType);
    if (!doc->usages.empty()
        && std::ranges::none_of(doc->usages, [&](const auto& usage) {
             return matchesUsageCategory(element, usage);
           })) {
      accept
          .warning(*attribute,
                   "This annotation is disallowed for element "
                   "of type "
                       + std::string(attributeUsageName(element)) + ".")
          .data(diagnosticData(InvalidAttribute));
    }

    if (!doc->allowMultiple && !visited.insert(attributeType).second) {
      accept
          .warning(*attribute,
                   "Duplicated annotation of a non-repeatable type. Only "
                   "annotation types marked with '@allowMultiple' can be "
                   "used multiple times on a single target.")
          .data(diagnosticData(InvalidAttribute));
    }
  }
}

std::shared_ptr<const XsmpcatValidator::AttributeTypeDoc>
XsmpcatValidator::attributeTypeDoc(const AttributeType& type) const {
  return _attributeTypeDocs.get(documentOf(type).id, &type, [&type] {
    auto doc = std::make_shared<AttributeTypeDoc>();
    for (auto& usage : docTagRanges(type, "usage"))
      doc->usages.push_back(std::move(usage.content));
    doc->allowMultiple = docTag(type, "allowMultiple").has_value();
    return std::shared_ptr<const AttributeTypeDoc>(std::move(doc));
  });
}

void XsmpcatValidator::checkNamedElement(const NamedElement& element,
                                         Acceptor& accept) const {
  // An empty name is tolerated here (some grammar positions never require one);
  // only a non-empty malformed name or a reserved keyword is flagged.
  if (!element.name.empty()) {
    if (!matchesNamePattern(element.name)) {
      accept.error(element, "A name shall start with a letter.")
          .property<&NamedElement::name>();
    }
    if (isCppKeyword(element.name)) {
      accept.error(element, "A name shall not be an ISO/ANSI C++ keyword.")
          .property<&NamedElement::name>();
    }
  }
  checkAttributeUsages(element, accept);
}

void XsmpcatValidator::checkEnumeration(const Enumeration& enumeration,
                                        Acceptor& accept) const {
  checkModifiers(enumeration, {&isVisibilityModifier}, accept);
  if (enumeration.literal.empty()) {
    accept
        .error(enumeration,
               "An Enumeration shall contain at least one literal.")
        .keyword("enum");
  }

  std::unordered_set<std::string_view> names;
  std::unordered_set<std::int64_t> values;
  for (const auto* literal : enumeration.literal) {
    if (literal == nullptr) {
      continue;
    }
    if (!literal->name.empty() && !names.insert(literal->name).second) {
      accept.error(*literal, "Duplicated literal name.")
          .property<&NamedElement::name>();
    }
    if (const auto value = getValueAs(literal->value, PTK::Int32, &accept)) {
      if (const auto key = integralValueOf(*value)) {
        if (!values.insert(*key).second) {
          accept
              .error(*literal,
                     "Enumeration Literal Values shall be unique "
                     "within an Enumeration.")
              .property<&EnumerationLiteral::value>();
        }
      }
    }
  }
}

void XsmpcatValidator::checkInteger(const Integer& integer,
                                    Acceptor& accept) const {
  checkModifiers(integer, {&isVisibilityModifier}, accept);
  if (integer.primitiveType.has_value()) {
    if (const auto* type = integer.primitiveType->get(); type != nullptr) {
      checkTypeReferenceExtras<&Integer::primitiveType>(integer, *type, accept);
    }
  }

  const auto kind = getPTK(&integer);
  if (!isIntegerKind(kind)) {
    accept.error(integer, "Expecting an Integral Type.")
        .property<&Integer::primitiveType>();
    return;
  }

  const auto min = getValueAs(integer.minimum, kind, &accept);
  const auto max = getValueAs(integer.maximum, kind, &accept);
  if (min && max) {
    const auto minValue = integralValueOf(*min);
    const auto maxValue = integralValueOf(*max);
    if (minValue && maxValue
        && (isSignedIntegral(kind) ? *minValue > *maxValue
                                   : min->integral > max->integral)) {
      accept.error(integer, "Minimum shall be less or equal than Maximum.")
          .property<&Integer::minimum>();
    }
  }
  // A type registers its bounds as Int64.
  if (kind == PTK::UInt64
      && (integer.maximum == nullptr
          || (max
              && max->integral > static_cast<std::uint64_t>(
                     std::numeric_limits<std::int64_t>::max())))) {
    constexpr auto message =
        "The UInt64 upper bound exceeds Int64_MAX (9223372036854775807), the "
        "limit supported by SMP type registration.";
    if (integer.maximum != nullptr)
      accept.warning(integer, message).property<&Integer::maximum>();
    else
      accept.warning(integer, message).property<&NamedElement::name>();
  }
}

void XsmpcatValidator::checkFloat(const Float& floatType,
                                  Acceptor& accept) const {
  checkModifiers(floatType, {&isVisibilityModifier}, accept);
  if (floatType.primitiveType.has_value()) {
    if (const auto* type = floatType.primitiveType->get(); type != nullptr) {
      checkTypeReferenceExtras<&Float::primitiveType>(floatType, *type, accept);
    }
  }

  const auto kind = getPTK(&floatType);
  if (!isFloatingType(kind)) {
    accept.error(floatType, "Expecting a Floating Point Type.")
        .property<&Float::primitiveType>();
    return;
  }

  const auto min = getValueAs(floatType.minimum, kind, &accept);
  const auto max = getValueAs(floatType.maximum, kind, &accept);
  if (min && max) {
    const auto minValue = floatingValueOf(*min);
    const auto maxValue = floatingValueOf(*max);
    if (minValue && maxValue) {
      const auto inclusive = floatType.range == "...";
      if (*minValue > *maxValue || (*minValue == *maxValue && !inclusive)) {
        accept
            .error(floatType,
                   inclusive ? "Minimum shall be less or equal than Maximum."
                             : "Minimum shall be less than Maximum.")
            .property<&Float::minimum>();
      }
    }
  }
}

void XsmpcatValidator::checkConstant(const Constant& constant,
                                     Acceptor& accept) const {
  const auto* type = constant.type.get();
  if (type == nullptr) {
    return;
  }
  checkTypeReferenceExtras<&Constant::type>(constant, *type, accept);
  if (constant.value == nullptr) {
    accept.error(constant, "A Constant must have an initialization value.")
        .keyword("constant")
        .data(diagnosticData(MissingValue));
    return;
  }
  if (isNullptr(constant.value)) {
    accept.error(*constant.value, nullptrNeedsAPointer);
    return;
  }
  checkValueBounds(*type, constant.value, accept);
}

void XsmpcatValidator::checkStringType(const String& stringType,
                                       Acceptor& accept) const {
  checkModifiers(stringType, {&isVisibilityModifier}, accept);
  if (stringType.length == nullptr) {
    accept.error(stringType, "Missing String length.").keyword("string");
    return;
  }
  if (const auto value = getValueAs(stringType.length, PTK::Int64, &accept)) {
    if (const auto length = integralValueOf(*value); length && *length < 0) {
      accept.error(stringType, "The String length shall be a positive number.")
          .property<&String::length>();
    }
  }
}

void XsmpcatValidator::checkArrayType(const Array& arrayType,
                                      Acceptor& accept) const {
  checkModifiers(arrayType, {&isVisibilityModifier}, accept);
  if (arrayType.size == nullptr) {
    accept.error(arrayType, "Missing Array size.").keyword("array");
  } else if (const auto size = getValueAs(arrayType.size, PTK::Int64, &accept);
             size && integralValueOf(*size) <= 0) {
    accept.error(arrayType, "The Array size shall be a positive number.")
        .property<&Array::size>();
  }

  if (const auto* itemType = arrayType.itemType.get()) {
    checkTypeReferenceExtras<&Array::itemType>(arrayType, *itemType, accept);
    if (isRecursiveMember(arrayType, itemType)) {
      accept.error(arrayType, "Recursive Array Type.")
          .property<&Array::itemType>();
    }
    if (cursiv::ast_cast<const SimpleType>(itemType) == nullptr
        && isSimpleArrayAttribute(arrayType)) {
      accept
          .error(arrayType,
                 "An array annotated with '@SimpleArray' requires "
                 "a SimpleType item type.")
          .property<&Array::itemType>()
          .data(diagnosticData(NonSimpleArray));
    }
  }
}

void XsmpcatValidator::checkStructure(const Structure& structure,
                                      Acceptor& accept) const {
  checkModifiers(structure, {&isVisibilityModifier}, accept);
  checkDuplicatedMembers(structure, structure.elements, accept);
  for (const auto* element : structure.elements) {
    if (element == nullptr) {
      continue;
    }
    checkMemberModifiers(*element, Owner::Structure, accept);
    if (const auto* field = cursiv::ast_cast<const Field>(element)) {
      if (isRecursiveMember(structure, field->type.get())) {
        accept.error(*field, "Recursive Field Type.").property<&Field::type>();
      }
    }
  }
}

void XsmpcatValidator::checkClass(const Class& clazz, Acceptor& accept) const {
  checkModifiers(clazz, {&isVisibilityModifier, &isAbstractModifier}, accept);
  if (clazz.base.has_value()) {
    if (const auto* base = clazz.base->get(); base != nullptr) {
      checkTypeReferenceExtras<&Class::base>(clazz, *base, accept);
      if (isBaseOfClass(&clazz, base)) {
        accept.error(clazz, "Cyclic dependency detected.")
            .property<&Class::base>()
            .data(diagnosticData(CyclicClassBase));
      }
    }
  }

  checkDuplicatedMembers(clazz, clazz.elements, accept);
  for (const auto* element : clazz.elements) {
    if (element == nullptr) {
      continue;
    }
    checkMemberModifiers(*element, Owner::Class, accept);
    if (const auto* field = cursiv::ast_cast<const Field>(element)) {
      if (isRecursiveMember(clazz, field->type.get())) {
        accept.error(*field, "Recursive Field Type.").property<&Field::type>();
      }
    }
  }

  const auto hasAbstractModifier =
      std::ranges::find(clazz.modifiers, "abstract") != clazz.modifiers.end();
  if (!hasAbstractModifier && hasAbstractMember(clazz.elements)) {
    const auto isException =
        cursiv::ast_cast<const Exception>(&clazz) != nullptr;
    accept
        .warning(clazz, isException ? "The Exception shall be abstract."
                                    : "The Class shall be abstract.")
        .keyword(isException ? "exception" : "class")
        .data(diagnosticData(MissingAbstract));
  }
}

void XsmpcatValidator::checkInterface(const Interface& interface,
                                      Acceptor& accept) const {
  checkModifiers(interface, {&isVisibilityModifier}, accept);

  std::unordered_set<const Type*> bases;
  for (std::size_t i = 0; i < interface.base.size(); ++i) {
    const auto* base = interface.base[i].get();
    if (base == nullptr) {
      continue;
    }
    checkTypeReferenceExtras<&Interface::base>(interface, *base, accept, i);
    if (!bases.insert(base).second) {
      accept.error(interface, "Duplicated interface.")
          .property<&Interface::base>(i)
          .data(diagnosticData(DuplicatedInterfaceBase));
    } else if (isBaseOfInterface(&interface, base)) {
      accept.error(interface, "Cyclic dependency detected.")
          .property<&Interface::base>(i)
          .data(diagnosticData(CyclicInterfaceBase));
    }
  }

  checkDuplicatedMembers(interface, interface.elements, accept);
  for (const auto* element : interface.elements) {
    if (element != nullptr) {
      checkMemberModifiers(*element, Owner::Interface, accept);
    }
  }
}

void XsmpcatValidator::checkComponent(const Component& component,
                                      Acceptor& accept) const {
  checkModifiers(component, {&isVisibilityModifier, &isAbstractModifier},
                 accept);
  if (component.base.has_value()) {
    if (const auto* base = component.base->get(); base != nullptr) {
      checkTypeReferenceExtras<&Component::base>(component, *base, accept);
      if (isBaseOfComponent(&component, base)) {
        accept.error(component, "Cyclic dependency detected.")
            .property<&Component::base>()
            .data(diagnosticData(CyclicComponentBase));
      }
    }
  }

  std::unordered_set<const Type*> interfaces;
  for (std::size_t i = 0; i < component.interface_.size(); ++i) {
    const auto* base = component.interface_[i].get();
    if (base == nullptr) {
      continue;
    }
    checkTypeReferenceExtras<&Component::interface_>(component, *base, accept,
                                                     i);
    if (!interfaces.insert(base).second) {
      accept.error(component, "Duplicated interface.")
          .property<&Component::interface_>(i)
          .data(diagnosticData(DuplicatedComponentInterface));
    }
  }

  checkDuplicatedMembers(component, component.elements, accept);
  for (const auto* element : component.elements) {
    if (element != nullptr) {
      checkMemberModifiers(*element, Owner::Component, accept);
    }
  }

  const auto hasAbstractModifier =
      std::ranges::find(component.modifiers, "abstract")
      != component.modifiers.end();
  if (!hasAbstractModifier && hasAbstractMember(component.elements)) {
    const auto isModel = cursiv::ast_cast<const Model>(&component) != nullptr;
    accept
        .warning(component, isModel ? "The Model shall be abstract."
                                    : "The Service shall be abstract.")
        .keyword(isModel ? "model" : "service")
        .data(diagnosticData(MissingAbstract));
  }
}

namespace {

template <typename T>
bool isA(const Type* type) {
  return cursiv::ast_cast<const T>(type) != nullptr;
}

// The non-static fields of `structure`, base chain first (recursively) then
// `structure`'s own — UNFILTERED by visibility (unlike `xsmp::getAllFields`,
// which also requires `public`: that one lists the fields a collection literal
// addresses by position; this one supports `findNonValueTypeField`'s recursive
// composition check, which cares about every stored field regardless of
// visibility).
std::vector<const Field*> getAllNonStaticFields(
    const Structure& structure, std::unordered_set<const Structure*>& visited) {
  if (!visited.insert(&structure).second) {
    return {};
  }
  std::vector<const Field*> result;
  if (const auto* clazz = cursiv::ast_cast<const Class>(&structure)) {
    if (clazz->base.has_value()) {
      if (const auto* base =
              cursiv::ast_cast<const Structure>(clazz->base->get())) {
        result = getAllNonStaticFields(*base, visited);
      }
    }
  }
  for (const auto* element : structure.elements) {
    if (const auto* field = cursiv::ast_cast<const Field>(element);
        field != nullptr && !isStaticAttribute(*field)) {
      result.push_back(field);
    }
  }
  return result;
}

// The first Field, reachable (recursively) through `type`'s own Structure/Class
// fields or an ArrayType's item type, whose OWN declared type isn't a ValueType
// — or nullptr when every reachable field is ValueType-typed. For a type in a
// cycle only: `fieldSummary` reads the others once each.
const Field* findNonValueTypeField(const Type* type,
                                   std::unordered_set<const Type*>& visited) {
  if (type == nullptr || !visited.insert(type).second) {
    return nullptr;
  }
  if (const auto* arrayType = cursiv::ast_cast<const Array>(type)) {
    return findNonValueTypeField(arrayType->itemType.get(), visited);
  }
  const auto* structure = cursiv::ast_cast<const Structure>(type);
  if (structure == nullptr) {
    return nullptr;
  }
  std::unordered_set<const Structure*> fieldVisited;
  for (const auto* field : getAllNonStaticFields(*structure, fieldVisited)) {
    const auto* fieldType = field->type.get();
    if (fieldType != nullptr && !isA<ValueType>(fieldType)) {
      return field;
    }
    if (const auto* invalid = findNonValueTypeField(fieldType, visited)) {
      return invalid;
    }
  }
  return nullptr;
}

// Whether `type` is `Smp.String8`, or an array of it at any depth.
bool resolvesToString8(const Type* type) {
  std::unordered_set<const Type*> visited;
  while (const auto* array = cursiv::ast_cast<const Array>(type)) {
    if (!visited.insert(type).second) return false;
    type = array->itemType.get();
  }
  return isString8(type);
}

// The first field whose type resolves to String8, reached through `type`'s
// structures and array items, or null. Fields not of a ValueType are not
// followed.
const Field* findString8Field(const Type* type,
                              std::unordered_set<const Type*>& visited) {
  if (!type || !visited.insert(type).second) return nullptr;
  if (const auto* array = cursiv::ast_cast<const Array>(type))
    return findString8Field(array->itemType.get(), visited);
  const auto* structure = cursiv::ast_cast<const Structure>(type);
  if (!structure) return nullptr;
  std::unordered_set<const Structure*> fieldVisited;
  for (const auto* field : getAllNonStaticFields(*structure, fieldVisited)) {
    const auto* fieldType = field->type.get();
    if (!fieldType || !isA<ValueType>(fieldType)) continue;
    if (resolvesToString8(fieldType)) return field;
    if (const auto* nested = findString8Field(fieldType, visited))
      return nested;
  }
  return nullptr;
}

// Walks the graph under `root`, children first, and calls `done` on each node
// that is not `known` yet. False when the graph has a cycle: a node of the
// cycle, and every node above it, is never done.
template <class Known, class Children, class Done>
bool walkAcyclic(const Type& root, const Known& known, const Children& children,
                 const Done& done) {
  if (known(root)) return true;

  struct Frame {
    const Type* type;
    std::vector<const Type*> children;
    std::size_t next = 0;
  };

  std::unordered_set<const Type*> active{&root};
  std::vector<Frame> stack;
  stack.push_back({&root, children(root)});
  while (!stack.empty()) {
    if (auto& frame = stack.back(); frame.next < frame.children.size()) {
      const auto* child = frame.children[frame.next++];
      if (known(*child)) continue;
      if (!active.insert(child).second) return false;
      stack.push_back({child, children(*child)});
    } else {
      done(*frame.type);
      active.erase(frame.type);
      stack.pop_back();
    }
  }
  return true;
}

// The types that the fields of `type` hold, with its base, or its item type.
std::vector<const Type*> fieldTypes(const Type& type, bool staticFields) {
  std::vector<const Type*> types;
  if (const auto* array = cursiv::ast_cast<const Array>(&type)) {
    if (const auto* item = array->itemType.get()) types.push_back(item);
    return types;
  }
  const auto* structure = cursiv::ast_cast<const Structure>(&type);
  if (!structure) return types;
  if (const auto* clazz = cursiv::ast_cast<const Class>(structure);
      clazz && clazz->base.has_value())
    if (const auto* base =
            cursiv::ast_cast<const Structure>(clazz->base->get()))
      types.push_back(base);
  for (const auto* element : structure->elements)
    if (const auto* field = cursiv::ast_cast<const Field>(element);
        field && field->type.get()
        && (staticFields || !isStaticAttribute(*field)))
      types.push_back(field->type.get());
  return types;
}

// Range/length diagnostics for a value `expr` against its declared `type`, PLUS
// recursive shape validation for an ArrayType/Structure target: `expr` must be
// a CollectionLiteral, whose elements are checked positionally against the item
// type / `getAllFields`, with size-mismatch error/warning.
// `getValueAs`/`integral|floatingValueOf` return nullopt when the expression is
// not a compile-time constant — those are reported by `getValueAs` itself when
// an acceptor is given.
void checkValueBounds(const Type& type, const Expression* expr,
                      Acceptor& accept) {
  if (expr == nullptr) {
    return;
  }
  if (isNullptr(expr)) {
    if (!isPointerType(&type)) accept.error(*expr, nullptrNeedsAPointer);
    return;
  }
  if (const auto* arrayType = cursiv::ast_cast<const Array>(&type)) {
    const auto* collection = cursiv::ast_cast<const CollectionLiteral>(expr);
    if (collection == nullptr) {
      accept.error(*expr, "An array shall be initialized with a collection.");
      return;
    }

    const auto arraySize = getValueAs(arrayType->size, PTK::UInt64);
    const auto sizeValue =
        arraySize ? integralValueOf(*arraySize) : std::nullopt;
    // A size that is not usable is `checkArrayType`'s to report.
    if (!sizeValue || *sizeValue < 0) {
      return;
    }

    const auto expectedSize = static_cast<std::size_t>(*sizeValue);
    const auto& elements = collection->elements;
    const auto checkCount = std::min(elements.size(), expectedSize);
    if (const auto* itemType = arrayType->itemType.get(); itemType != nullptr) {
      for (std::size_t i = 0; i < checkCount; ++i) {
        checkValueBounds(*itemType, elements[i], accept);
      }
    }

    if (elements.size() > expectedSize) {
      accept.error(*elements[expectedSize],
                   "The array type expect " + std::to_string(expectedSize)
                       + " element(s), got " + std::to_string(elements.size())
                       + " element(s).");
    } else if (elements.size() < expectedSize && !elements.empty()) {
      accept.warning(*expr,
                     "Partial initialization, the array type expect "
                         + std::to_string(expectedSize) + " element(s), got "
                         + std::to_string(elements.size()) + " element(s).");
    }
    return;
  }
  if (const auto* structure = cursiv::ast_cast<const Structure>(&type)) {
    const auto* collection = cursiv::ast_cast<const CollectionLiteral>(expr);
    if (collection == nullptr) {
      const auto isException =
          cursiv::ast_cast<const Exception>(structure) != nullptr;
      const auto isClass =
          !isException && cursiv::ast_cast<const Class>(structure) != nullptr;
      accept.error(*expr, std::string("A ")
                              + (isException ? "Exception"
                                 : isClass   ? "Class"
                                             : "Structure")
                              + " shall be initialized with a collection.");
      return;
    }

    std::unordered_set<const Structure*> visitedFields;
    const auto fields = getAllFields(*structure, visitedFields);
    const auto& elements = collection->elements;
    const auto checkCount = std::min(elements.size(), fields.size());
    for (std::size_t i = 0; i < checkCount; ++i) {
      const Expression* element = elements[i];
      const auto* field = fields[i];
      // An initializer may name its field, but the fields still come in order.
      if (const auto* designated =
              cursiv::ast_cast<const DesignatedInitializer>(element)) {
        if (designated->field.get() != field) {
          accept
              .error(*designated, "Invalid field name, expecting "
                                      + std::string(field->name) + ".")
              .property<&DesignatedInitializer::field>()
              .data(diagnosticData(InvalidFieldName));
        }
        element = designated->expr;
      }
      if (const auto* fieldType = field->type.get();
          fieldType != nullptr
          && (isA<ValueType>(fieldType) || isNullptr(element))) {
        checkValueBounds(*fieldType, element, accept);
      }
    }

    if (elements.size() > fields.size()) {
      accept.error(*elements[fields.size()],
                   "The structure type expect " + std::to_string(fields.size())
                       + " element(s), got " + std::to_string(elements.size())
                       + " element(s).");
    } else if (elements.size() < fields.size() && !elements.empty()) {
      accept.warning(*expr,
                     "Partial initialization, the structure type expect "
                         + std::to_string(fields.size()) + " element(s), got "
                         + std::to_string(elements.size()) + " element(s).");
    }
    return;
  }
  // Scalar conversion and bounds are shared with the deployment validators and
  // the other solver callers.
  (void)getValueAs(expr, type, &accept);
}

}  // namespace

XsmpcatValidator::FieldSummary XsmpcatValidator::fieldSummary(
    const Type& type) const {
  const auto graphs =
      _typeGraphs.get({}, [] { return std::make_shared<TypeGraphs>(); });
  const std::scoped_lock lock(graphs->mutex);
  auto& summaries = graphs->summaries;
  if (const auto found = summaries.find(&type); found != summaries.end())
    return found->second;
  if (const auto found = graphs->cyclicSummaries.find(&type);
      found != graphs->cyclicSummaries.end())
    return found->second;

  // Each type once, from the summaries of what it holds: a structure scans its
  // base, then its own fields in order, as `getAllNonStaticFields` lists them.
  const auto summarize = [&](const Type& node) {
    FieldSummary summary;
    if (const auto* array = cursiv::ast_cast<const Array>(&node)) {
      if (const auto* item = array->itemType.get())
        summary = summaries.at(item);
      return summary;
    }
    const auto* structure = cursiv::ast_cast<const Structure>(&node);
    if (!structure) {
      summary.isString8 = isString8(&node);
      return summary;
    }
    if (const auto* clazz = cursiv::ast_cast<const Class>(structure);
        clazz && clazz->base.has_value())
      if (const auto* base =
              cursiv::ast_cast<const Structure>(clazz->base->get()))
        summary = summaries.at(base);
    summary.isString8 = false;
    for (const auto* element : structure->elements) {
      const auto* field = cursiv::ast_cast<const Field>(element);
      if (!field || isStaticAttribute(*field)) continue;
      const auto* fieldType = field->type.get();
      if (!fieldType) continue;
      if (!isA<ValueType>(fieldType)) {
        if (!summary.nonValueTypeField) summary.nonValueTypeField = field;
        continue;
      }
      const auto& nested = summaries.at(fieldType);
      if (nested.isString8) {
        if (!summary.string8Field) summary.string8Field = field;
      } else {
        if (!summary.nonValueTypeField)
          summary.nonValueTypeField = nested.nonValueTypeField;
        if (!summary.string8Field) summary.string8Field = nested.string8Field;
      }
    }
    return summary;
  };
  if (walkAcyclic(
          type, [&](const Type& node) { return summaries.contains(&node); },
          [](const Type& node) { return fieldTypes(node, false); },
          [&](const Type& node) { summaries.emplace(&node, summarize(node)); }))
    return summaries.at(&type);

  // In a cycle, what a walk meets first depends on where it starts.
  std::unordered_set<const Type*> nonValueVisited, string8Visited;
  const FieldSummary summary{
      .nonValueTypeField = findNonValueTypeField(&type, nonValueVisited),
      .string8Field = findString8Field(&type, string8Visited),
      .isString8 = resolvesToString8(&type)};
  return graphs->cyclicSummaries.emplace(&type, summary).first->second;
}

bool XsmpcatValidator::isRecursiveMember(const Type& owner,
                                         const Type* type) const {
  if (!type) return false;
  if (type == &owner) return true;
  {
    // With no cycle below `type`, nothing there leads back to `owner`, which
    // leads to `type`.
    const auto graphs =
        _typeGraphs.get({}, [] { return std::make_shared<TypeGraphs>(); });
    const std::scoped_lock lock(graphs->mutex);
    auto& acyclic = graphs->acyclic;
    if (walkAcyclic(
            *type, [&](const Type& node) { return acyclic.contains(&node); },
            [](const Type& node) { return fieldTypes(node, true); },
            [&](const Type& node) { acyclic.insert(&node); }))
      return false;
  }
  return isRecursiveType(&owner, type);
}

void XsmpcatValidator::checkField(const Field& field, Acceptor& accept) const {
  const auto* type = field.type.get();
  if (type == nullptr) {
    return;
  }
  checkTypeReferenceExtras<&Field::type>(field, *type, accept);

  if (cursiv::ast_cast<const Component>(field.parent) != nullptr) {
    if (!isA<ValueType>(type)) {
      accept
          .error(field,
                 "A Field of a Model or Service shall have a ValueType "
                 "type.")
          .property<&Field::type>();
    } else {
      if (const auto* invalidField = fieldSummary(*type).nonValueTypeField) {
        std::vector<cursiv::DiagnosticRelatedInformation> related;
        if (invalidField->cstNodeId != cursiv::kNoNode) {
          const auto span = rangeForFeature(*invalidField, "name");
          related.push_back(
              {.uri = std::string(documentOf(*invalidField).uri),
               .message = "Non-ValueType field " + qualifiedName(*invalidField),
               .range = span});
        }
        accept
            .error(field,
                   "A Field of a Model or Service shall not use a type "
                   "containing non-ValueType fields.")
            .property<&Field::type>()
            .relatedInformation(
                std::span<const cursiv::DiagnosticRelatedInformation>(related));
      }
    }
  } else if (!isA<ValueType>(type)) {
    accept
        .warning(field,
                 "Field type is not a ValueType. This is allowed "
                 "outside Model/Service but is not SMP compatible.")
        .property<&Field::type>();
  }

  if (isA<ValueType>(type)) {
    const bool component = cursiv::ast_cast<const Component>(field.parent);
    const auto summary = fieldSummary(*type);
    const auto direct = summary.isString8;
    const auto* nested = direct ? nullptr : summary.string8Field;
    if (direct || nested) {
      const std::string message =
          component
              ? (direct ? "A Field of a Model or Service shall not use the "
                          "String8 type."
                        : "A Field of a Model or Service shall not use a type "
                          "containing a String8 field.")
              : (direct
                     ? "Field type is String8. This is allowed outside "
                       "Model/Service but is not SMP compatible."
                     : "Field type contains a String8 field. This is allowed "
                       "outside Model/Service but is not SMP compatible.");
      auto diagnostic = component ? accept.error(field, message)
                                  : accept.warning(field, message);
      diagnostic.property<&Field::type>();
      if (nested) {
        const auto* document = nested->getDocument();
        if (document && nested->cstNodeId != cursiv::kNoNode) {
          const auto [begin, end] = rangeForFeature(*nested, "name");
          const cursiv::DiagnosticRelatedInformation related{
              .uri = std::string(document->uri),
              .message = "String8 field " + qualifiedName(*nested),
              .range = {.begin = begin, .end = end}};
          diagnostic.relatedInformation({&related, 1});
        }
      }
    }
  }

  if (isA<ValueType>(type) || isNullptr(field.default_)) {
    checkValueBounds(*type, field.default_, accept);
  }
}

void XsmpcatValidator::checkProperty(const Property& property,
                                     Acceptor& accept) const {
  if (const auto* type = property.type.get(); type != nullptr) {
    checkTypeReferenceExtras<&Property::type>(property, *type, accept);
    // The attached field's own checks (and the type-mismatch comparison) only
    // run once property.type resolved.
    if (property.attachedField.has_value()) {
      if (const auto* field = property.attachedField->get(); field != nullptr) {
        checkFieldReferenceExtras<&Property::attachedField>(property, *field,
                                                            accept);
        // A field of a class derived from the property's type fits as well.
        if (type != field->type.get()
            && !isBaseOfClass(cursiv::ast_cast<const Class>(type),
                              field->type.get())) {
          accept
              .error(property,
                     "The Type of the AttachedField shall match the Type "
                     "of the Property.")
              .property<&Property::attachedField>();
        }
      }
    }
  }

  std::unordered_set<const Type*> getRaises;
  for (std::size_t i = 0; i < property.getRaises.size(); ++i) {
    const auto* exception = property.getRaises[i].get();
    if (exception == nullptr) {
      continue;
    }
    checkTypeReferenceExtras<&Property::getRaises>(property, *exception, accept,
                                                   i);
    if (!getRaises.insert(exception).second) {
      accept.error(property, "Duplicated exception.")
          .property<&Property::getRaises>(i)
          .data(diagnosticData(DuplicatedException));
    }
  }

  std::unordered_set<const Type*> setRaises;
  for (std::size_t i = 0; i < property.setRaises.size(); ++i) {
    const auto* exception = property.setRaises[i].get();
    if (exception == nullptr) {
      continue;
    }
    checkTypeReferenceExtras<&Property::setRaises>(property, *exception, accept,
                                                   i);
    if (!setRaises.insert(exception).second) {
      accept.error(property, "Duplicated exception.")
          .property<&Property::setRaises>(i)
          .data(diagnosticData(DuplicatedException));
    }
  }

  if (const auto* isStatic = findAttribute(property, "Attributes.Static");
      attributeBoolValueOf(isStatic).value_or(false)) {
    if (cursiv::ast_cast<const Interface>(property.parent) != nullptr) {
      accept.error(*isStatic, "A Property of an Interface shall not be static.")
          .data(diagnosticData(InvalidAttribute));
    }
    if (const auto* isVirtual = findAttribute(property, "Attributes.Virtual");
        attributeBoolValueOf(isVirtual).value_or(false)) {
      accept
          .error(*isVirtual, "A Property shall not be both Static and Virtual.")
          .data(diagnosticData(InvalidAttribute));
    }
    if (const auto* isAbstract = findAttribute(property, "Attributes.Abstract");
        attributeBoolValueOf(isAbstract).value_or(false)) {
      accept
          .error(*isAbstract,
                 "A Property shall not be both Static and Abstract.")
          .data(diagnosticData(InvalidAttribute));
    }
    if (property.attachedField.has_value()) {
      if (const auto* field = property.attachedField->get();
          field != nullptr && !isStaticAttribute(*field)) {
        accept
            .error(*isStatic,
                   "A Property shall not be Static if the "
                   "attached field is not Static.")
            .data(diagnosticData(InvalidAttribute));
      }
    }
  }

  if (const auto* isByPointer = findAttribute(property, "Attributes.ByPointer");
      attributeBoolValueOf(isByPointer).value_or(false)
      && attributeBoolValue(property, "Attributes.ByReference")
             .value_or(false)) {
    accept
        .error(*isByPointer,
               "A Property shall not be both ByPointer and ByReference.")
        .data(diagnosticData(InvalidAttribute));
  }
}

void XsmpcatValidator::checkAssociation(const Association& association,
                                        Acceptor& accept) const {
  if (const auto* type = association.type.get(); type != nullptr) {
    checkTypeReferenceExtras<&Association::type>(association, *type, accept);
  }
}

void XsmpcatValidator::checkContainer(const Container& container,
                                      Acceptor& accept) const {
  // `defaultComponent` is only checked once `type` resolved.
  const auto* type = container.type.get();
  if (type == nullptr) {
    return;
  }
  checkTypeReferenceExtras<&Container::type>(container, *type, accept);
  if (!container.defaultComponent.has_value()) {
    return;
  }
  const auto* defaultComponent = container.defaultComponent->get();
  if (defaultComponent == nullptr) {
    return;
  }
  checkTypeReferenceExtras<&Container::defaultComponent>(
      container, *defaultComponent, accept);
  const auto fullName = qualifiedName(*type);
  if (!isBaseOfReferenceType(type, fullName, defaultComponent)) {
    accept
        .error(container,
               "The default Component shall be a sub type of " + fullName)
        .property<&Container::defaultComponent>();
  }
}

void XsmpcatValidator::checkReference(const Reference& reference,
                                      Acceptor& accept) const {
  const auto* type = reference.interface_.get();
  if (type == nullptr) {
    return;
  }
  checkTypeReferenceExtras<&Reference::interface_>(reference, *type, accept);
  if (getSmpStandard(reference) == "ECSS_SMP_2020" && !isA<Interface>(type)) {
    accept
        .error(reference,
               "A Reference in ECSS_SMP_2020 shall target an Interface.")
        .property<&Reference::interface_>();
  }
}

void XsmpcatValidator::checkEventSink(const EventSink& eventSink,
                                      Acceptor& accept) const {
  if (const auto* type = eventSink.type.get(); type != nullptr) {
    checkTypeReferenceExtras<&EventSink::type>(eventSink, *type, accept);
  }
}

void XsmpcatValidator::checkEventSource(const EventSource& eventSource,
                                        Acceptor& accept) const {
  if (const auto* type = eventSource.type.get(); type != nullptr) {
    checkTypeReferenceExtras<&EventSource::type>(eventSource, *type, accept);
  }
}

void XsmpcatValidator::checkValueReference(const ValueReference& valueReference,
                                           Acceptor& accept) const {
  checkModifiers(valueReference, {&isVisibilityModifier}, accept);
  if (const auto* type = valueReference.type.get(); type != nullptr) {
    checkTypeReferenceExtras<&ValueReference::type>(valueReference, *type,
                                                    accept);
  }
}

void XsmpcatValidator::checkEventType(const EventType& eventType,
                                      Acceptor& accept) const {
  checkModifiers(eventType, {&isVisibilityModifier}, accept);
  if (eventType.eventArgs.has_value()) {
    if (const auto* type = eventType.eventArgs->get(); type != nullptr) {
      checkTypeReferenceExtras<&EventType::eventArgs>(eventType, *type, accept);
    }
  }
}

void XsmpcatValidator::checkAttributeType(const AttributeType& attributeType,
                                          Acceptor& accept) const {
  checkModifiers(attributeType, {&isVisibilityModifier}, accept);
  // The standard's own catalogue declares a few without one, and is not the
  // user's to change.
  if (attributeType.default_ == nullptr
      && !isBuiltinUri(documentOf(attributeType).uri)) {
    accept.error(attributeType, "An AttributeType shall have a default value.")
        .keyword("attribute");
  }
  if (const auto* type = attributeType.type.get(); type != nullptr) {
    checkTypeReferenceExtras<&AttributeType::type>(attributeType, *type,
                                                   accept);
    checkAttributeValue(*type, attributeType.default_, accept);
    std::unordered_set<std::string> visitedUsages;
    for (const auto& usage : docTagRanges(attributeType, "usage")) {
      if (!validUsageCategories().contains(usage.content)) {
        accept
            .warning(attributeType,
                     "Unknown usage '" + utf8Text(usage.content) + "'.")
            .range({.begin = usage.begin, .end = usage.end})
            .data(diagnosticData(InvalidUsage));
      }
      if (!visitedUsages.insert(usage.content).second) {
        accept.warning(attributeType, "Duplicated usage.")
            .range({.begin = usage.begin, .end = usage.end})
            .data(diagnosticData(DuplicatedUsage));
      }
    }
  }
}

void XsmpcatValidator::checkNativeType(const NativeType& nativeType,
                                       Acceptor& accept) const {
  checkModifiers(nativeType, {&isVisibilityModifier}, accept);
  if (!docTag(nativeType, "type").has_value()) {
    accept
        .error(nativeType,
               "The javadoc '@type' tag shall be defined with the C++ type "
               "name.")
        .property<&NamedElement::name>();
  }
}

void XsmpcatValidator::checkPrimitiveType(const PrimitiveType& primitiveType,
                                          Acceptor& accept) const {
  checkModifiers(primitiveType, {&isVisibilityModifier}, accept);
  if (getPTK(&primitiveType) == PTK::None) {
    accept.error(primitiveType, "Unsupported Primitive Type.")
        .property<&NamedElement::name>();
  }
}

namespace {

// The keyword introducing a Type's concrete grammar rule, used to anchor
// "Missing Type UUID" on the type declaration's own keyword rather than the
// whole node. Exception is checked before Class (Exception extends Class, so
// `isA<Class>` also matches it).
std::string_view keywordForType(const Type& type) {
  if (isA<Array>(&type)) {
    return "array";
  }
  if (isA<AttributeType>(&type)) {
    return "attribute";
  }
  if (isA<Exception>(&type)) {
    return "exception";
  }
  if (isA<Class>(&type)) {
    return "class";
  }
  if (isA<Enumeration>(&type)) {
    return "enum";
  }
  if (isA<EventType>(&type)) {
    return "event";
  }
  if (isA<Float>(&type)) {
    return "float";
  }
  if (isA<Integer>(&type)) {
    return "integer";
  }
  if (isA<Interface>(&type)) {
    return "interface";
  }
  if (isA<Model>(&type)) {
    return "model";
  }
  if (isA<NativeType>(&type)) {
    return "native";
  }
  if (isA<PrimitiveType>(&type)) {
    return "primitive";
  }
  if (isA<Service>(&type)) {
    return "service";
  }
  if (isA<String>(&type)) {
    return "string";
  }
  if (isA<Structure>(&type)) {
    return "struct";
  }
  if (isA<ValueReference>(&type)) {
    return "using";
  }
  return {};
}

// A UUID is a number, so its case does not count.
std::string foldedUuid(std::string_view uuid) {
  std::string folded(uuid);
  for (auto& c : folded)
    if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
  return folded;
}

// The documents whose text holds a `@uuid` that a document of the user also
// writes. A duplicate is rare and comments are costly to parse, so the text
// says which documents are worth parsing. The same text that is not a tag of
// two types only reads more. Built-ins are not checked: two of them may share
// one.
std::vector<cursiv::DocumentId> documentsSharingUuidText(
    const cursiv::Documents& documents) {
  constexpr std::string_view tag = "@uuid";
  constexpr std::size_t uuidSize = 36;

  struct Written {
    std::size_t uuid;
    bool builtin;
    cursiv::DocumentId document;
    auto operator<=>(const Written&) const = default;
  };

  std::vector<Written> written;
  for (const auto& document : documents.all()) {
    if (!cursiv::ast_cast<const Catalogue>(document->parseResult.value))
      continue;
    const std::string_view text = document->textDocument().getText();
    const auto builtin = isBuiltinUri(document->uri);
    for (auto at = text.find(tag); at != std::string_view::npos;
         at = text.find(tag, at + tag.size())) {
      // The UUID may stand on the next line of the comment.
      const auto begin = text.find_first_not_of(" \t\r\n*", at + tag.size());
      if (begin == std::string_view::npos || begin + uuidSize > text.size())
        break;
      written.push_back(
          {std::hash<std::string>{}(foldedUuid(text.substr(begin, uuidSize))),
           builtin, document->id});
    }
  }
  // By UUID, those of the user first.
  std::ranges::sort(written);
  std::vector<cursiv::DocumentId> suspects;
  for (auto same = written.begin(); same != written.end();) {
    const auto end = std::ranges::find_if(
        same, written.end(),
        [&](const Written& other) { return other.uuid != same->uuid; });
    if (end - same > 1 && !same->builtin)
      for (auto at = same; at != end; ++at) suspects.push_back(at->document);
    same = end;
  }
  std::ranges::sort(suspects);
  suspects.erase(std::ranges::unique(suspects).begin(), suspects.end());
  return suspects;
}

// Used by `computeCatalogueCycles` to find circular cross-document Catalogue
// dependencies.
std::vector<std::vector<cursiv::DocumentId>> computeStronglyConnectedComponents(
    const std::map<cursiv::DocumentId, std::vector<cursiv::DocumentId>>&
        graph) {
  using cursiv::DocumentId;
  std::size_t index = 0;
  std::vector<DocumentId> stack;
  std::unordered_map<DocumentId, std::size_t> indices;
  std::unordered_map<DocumentId, std::size_t> lowLinks;
  std::unordered_set<DocumentId> onStack;
  std::vector<std::vector<DocumentId>> components;

  // Tarjan's algorithm. A node whose low link is still its own index closes a
  // component: itself and everything above it on the stack.
  const std::function<void(DocumentId)> visit = [&](DocumentId node) {
    indices[node] = lowLinks[node] = index++;
    stack.push_back(node);
    onStack.insert(node);
    if (const auto it = graph.find(node); it != graph.end()) {
      for (const auto dependency : it->second) {
        if (!indices.contains(dependency)) {
          visit(dependency);
          lowLinks[node] = std::min(lowLinks[node], lowLinks[dependency]);
        } else if (onStack.contains(dependency)) {
          lowLinks[node] = std::min(lowLinks[node], indices[dependency]);
        }
      }
    }

    if (lowLinks[node] != indices[node]) {
      return;
    }
    std::vector<DocumentId> component;
    DocumentId current{};
    do {
      current = stack.back();
      stack.pop_back();
      onStack.erase(current);
      component.push_back(current);
    } while (current != node);
    components.push_back(std::move(component));
  };

  for (const auto& [node, dependencies] : graph) {
    (void)dependencies;
    if (!indices.contains(node)) {
      visit(node);
    }
  }
  return components;
}

}  // namespace

// A namespace is flagged when another element of the same document has its
// qualified name. checkType reports a Type that collides with a visible
// Namespace of another document.
void XsmpcatValidator::checkNamespace(const Namespace& ns,
                                      Acceptor& accept) const {
  if (ns.name.empty()) {
    return;
  }
  const auto& document = documentOf(ns);
  if (isBuiltinUri(document.uri)) {
    // The ECSS builtins reopen namespaces dozens of times in one file (`Smp`
    // 16 times in ecss.smp@ECSS_SMP_2020.xsmpcat), like the exemption of
    // checkType and checkCatalogue.
    return;
  }
  std::vector<cursiv::DiagnosticRelatedInformation> related;
  visitSameName(ns, [&](const cursiv::AstNode& candidate,
                        const cursiv::AstNodeDescription& description) {
    if (description.documentId != document.id
        || cursiv::ast_cast<const Catalogue>(&candidate)) {
      return;
    }
    const auto [begin, end] = rangeForFeature(candidate, "name", 0);
    related.push_back({.uri = std::string(documentOf(candidate).uri),
                       .message = std::string(description.name),
                       .range = {.begin = begin, .end = end}});
  });
  if (!related.empty()) {
    accept.error(ns, "Duplicated name.")
        .property<&NamedElement::name>()
        .relatedInformation(
            std::span<const cursiv::DiagnosticRelatedInformation>(related));
  }
}

void XsmpcatValidator::checkType(const Type& type, Acceptor& accept) const {
  const auto uuidTag = docTagRange(type, "uuid");
  if (!uuidTag.has_value()) {
    accept.error(type, "Missing Type UUID.")
        .keyword(keywordForType(type))
        .data(diagnosticData(MissingUuid));
  } else if (const auto& uuid = uuidTag->content; !isUUID(uuid)) {
    // Anchored on the `@uuid` tag's own text, not the whole Type declaration.
    // `.range()` is an absolute document offset pair.
    accept.error(type, "The UUID is invalid.")
        .range({.begin = uuidTag->begin, .end = uuidTag->end})
        .data(diagnosticData(InvalidUuid));
  } else if (!isBuiltinUri(documentOf(type).uri)) {
    // Duplicated UUID: workspace-wide, unfiltered by project visibility,
    // excluding builtins.
    const auto workspace = workspaceFacts();
    const auto& documents = *services.shared.documents;
    std::vector<cursiv::DiagnosticRelatedInformation> related;
    if (const auto same = workspace->duplicatedUuids.find(foldedUuid(uuid));
        same != workspace->duplicatedUuids.end())
      for (const auto& description : same->second) {
        const auto& candidate = resolveNode(documents, description);
        if (&candidate == &type) continue;
        const auto [begin, end] = rangeForFeature(candidate, "name", 0);
        related.push_back({.uri = std::string(documentOf(candidate).uri),
                           .message = std::string(description.name),
                           .range = {.begin = begin, .end = end}});
      }
    if (!related.empty()) {
      accept.error(type, "Duplicated UUID.")
          .range({.begin = uuidTag->begin, .end = uuidTag->end})
          .data(diagnosticData(DuplicatedUuid))
          .relatedInformation(
              std::span<const cursiv::DiagnosticRelatedInformation>(related));
    }
  }

  checkDuplicatedTypeName(type, accept);
}

std::shared_ptr<const XsmpcatValidator::WorkspaceFacts>
XsmpcatValidator::workspaceFacts() const {
  // The first document to ask reads them, and the others wait for that one.
  return _workspaceFacts.get({}, [this] {
    const std::scoped_lock lock(_workspaceFactsMutex);
    return _workspaceFacts.get({}, [this] { return readWorkspaceFacts(); });
  });
}

std::shared_ptr<const XsmpcatValidator::WorkspaceFacts>
XsmpcatValidator::readWorkspaceFacts() const {
  auto facts = std::make_shared<WorkspaceFacts>();
  const auto& documents = *services.shared.documents;
  std::map<cursiv::DocumentId, cursiv::AstNodeDescription> catalogues;
  (void)services.shared.indexManager->visitElements(
      nullptr, {}, [&](const cursiv::AstNodeDescription& description) {
        if (!description.type) return true;
        if (description.type->derivesFrom(&cursiv::ast_type_tag<Service>)) {
          const auto& node = resolveNode(documents, description);
          facts->servicesByName[cursiv::ast_cast<const Service>(&node)->name]
              .push_back(description);
        } else if (description.type->derivesFrom(
                       &cursiv::ast_type_tag<Catalogue>)) {
          // A built-in reads only built-ins, so it is in no cycle of the user.
          if (const auto document =
                  documents.getDocument(description.documentId);
              document && !isBuiltinUri(document->uri))
            catalogues.emplace(description.documentId, description);
        }
        return true;
      });

  const auto suspects = documentsSharingUuidText(documents);
  if (!suspects.empty()) {
    std::unordered_map<std::string, std::vector<cursiv::AstNodeDescription>>
        typesByUuid;
    (void)services.shared.indexManager->visitElements(
        &cursiv::ast_type_tag<Type>, suspects,
        [&](const cursiv::AstNodeDescription& description) {
          // A tag that is no UUID cannot have the UUID of another type.
          if (const auto tag =
                  docTag(resolveNode(documents, description), "uuid");
              tag && isUUID(*tag))
            typesByUuid[foldedUuid(*tag)].push_back(description);
          return true;
        });
    for (auto& [uuid, types] : typesByUuid)
      if (types.size() > 1)
        facts->duplicatedUuids.emplace(uuid, std::move(types));
  }

  facts->catalogueCycles = computeCatalogueCycles(catalogues);
  return std::shared_ptr<const WorkspaceFacts>(std::move(facts));
}

void XsmpcatValidator::checkDuplicatedTypeName(const Type& type,
                                               Acceptor& accept) const {
  if (type.name.empty()) {
    return;
  }
  std::vector<cursiv::DiagnosticRelatedInformation> related;
  visitSameName(type, [&](const cursiv::AstNode& candidate,
                          const cursiv::AstNodeDescription& description) {
    const auto [begin, end] = rangeForFeature(candidate, "name", 0);
    related.push_back({.uri = std::string(documentOf(candidate).uri),
                       .message = std::string(description.name),
                       .range = {.begin = begin, .end = end}});
  });
  if (!related.empty()) {
    accept.error(type, "Duplicated Type name.")
        .property<&NamedElement::name>()
        .data(diagnosticData(DuplicatedTypeName))
        .relatedInformation(
            std::span<const cursiv::DiagnosticRelatedInformation>(related));
  }
}

void XsmpcatValidator::visitSameName(
    const NamedElement& element,
    cursiv::function_ref<void(const cursiv::AstNode&,
                              const cursiv::AstNodeDescription&)>
        visitor) const {
  // The workspace index exports an element under its fully qualified name
  // (XsmpcatScopeComputation), not its simple `name`: match that key.
  const auto visible = services.shared.projectManager->getVisibleDocumentIds(
      documentOf(element));
  const auto& documents = *services.shared.documents;
  (void)services.shared.indexManager->visitByName(
      qualifiedName(element), true, {}, *services.astReflection,
      [&](const cursiv::AstNodeDescription& description) {
        if (visible->contains(description.documentId)) {
          const auto& candidate = resolveNode(documents, description);
          // Every exported NamedElement counts, whatever its kind.
          if (&candidate != &element
              && cursiv::ast_cast<const NamedElement>(&candidate))
            visitor(candidate, description);
        }
        return true;
      });
}

std::string XsmpcatValidator::getSmpStandard(
    const cursiv::AstNode& node) const {
  const auto& document = documentOf(node);
  if (const auto* project =
          services.shared.projectManager->getProject(document)) {
    return project->standard.empty() ? "ECSS_SMP_2020"
                                     : std::string(project->standard);
  }
  const auto tagged = builtinStandard(document.uri);
  return std::string(tagged.empty() ? "ECSS_SMP_2020" : tagged);
}

void XsmpcatValidator::checkCatalogue(const Catalogue& catalogue,
                                      Acceptor& accept) const {
  if (const auto date = docTagRange(catalogue, "date");
      date.has_value() && !parseDateTimeMilliseconds(date->content)) {
    // Anchored on the `@date` tag's own text: `.range()` takes absolute
    // document offsets.
    accept
        .warning(catalogue, "Invalid date format (e.g: 1970-01-01T00:00:00Z).")
        .range({.begin = date->begin, .end = date->end});
  }

  // Workspace-wide, unfiltered by project visibility, exempting builtins. Needs
  // the Catalogue exported to the global index by `XsmpcatScopeComputation`.
  if (!catalogue.name.empty() && !isBuiltinUri(documentOf(catalogue).uri)) {
    const auto& documents = *services.shared.documents;
    std::vector<cursiv::DiagnosticRelatedInformation> related;
    (void)services.shared.indexManager->visitByName(
        catalogue.name, true, {}, *services.astReflection,
        [&](const cursiv::AstNodeDescription& description) {
          if (!description.type
              || !description.type->derivesFrom(
                  &cursiv::ast_type_tag<Document>))
            return true;
          const auto& candidate = resolveNode(documents, description);
          if (&candidate == &catalogue) return true;
          const auto [begin, end] = rangeForFeature(candidate, "name", 0);
          related.push_back({.uri = std::string(documentOf(candidate).uri),
                             .message = std::string(description.name),
                             .range = {.begin = begin, .end = end}});
          return true;
        });
    if (!related.empty()) {
      accept.error(catalogue, "Duplicated Document name.")
          .property<&NamedElement::name>()
          .relatedInformation(
              std::span<const cursiv::DiagnosticRelatedInformation>(related));
    }
  }

  checkEncoding(catalogue, accept);
  checkEscapes(catalogue, accept);
  checkOutputFile(catalogue, accept);

  // Warn when this catalogue isn't reachable from any `xsmp.project` (a
  // builtin is exempt — it is always implicitly visible).
  const auto& document = documentOf(catalogue);
  if (!isBuiltinUri(document.uri)
      && services.shared.projectManager->getProject(document) == nullptr) {
    accept.warning(catalogue, "This Catalogue is not contained in a project.")
        .keyword("catalogue");
  }

  // Circular cross-document dependencies (Tarjan-SCC over the visible-
  // catalogue reference graph). Builtins are exempt (never form a cycle with
  // anything user-authored).
  if (!isBuiltinUri(document.uri)) {
    const auto workspace = workspaceFacts();
    if (const auto it = workspace->catalogueCycles.find(document.id);
        it != workspace->catalogueCycles.end()) {
      const auto& documents = *services.shared.documents;
      std::vector<cursiv::DiagnosticRelatedInformation> related;
      for (const auto& participant : it->second) {
        const auto& node = resolveNode(documents, participant);
        const auto [begin, end] = rangeForFeature(node, "name", 0);
        related.push_back({.uri = std::string(documentOf(node).uri),
                           .message = std::string(participant.name),
                           .range = {.begin = begin, .end = end}});
      }
      accept
          .error(catalogue, "Catalogues shall not have circular dependencies.")
          .keyword("catalogue")
          .relatedInformation(
              std::span<const cursiv::DiagnosticRelatedInformation>(related));
    }
  }
}

// The catalogue-dependency cycles of the workspace. An edge is "catalogue A has
// a resolved reference into catalogue B", read from each catalogue's
// `ReferenceDescriptionProvider::createDescriptions`, since xsmpcat has no
// import syntax. A lone self-referencing catalogue is never a cycle. A
// catalogue only reads what its project sees, and that holds what its
// dependencies see, so a cycle is the same for every project that sees one of
// its catalogues.
std::map<cursiv::DocumentId, std::vector<cursiv::AstNodeDescription>>
XsmpcatValidator::computeCatalogueCycles(
    const std::map<cursiv::DocumentId, cursiv::AstNodeDescription>& catalogues)
    const {
  using cursiv::AstNodeDescription;
  using cursiv::DocumentId;

  const auto& documents = *services.shared.documents;
  std::map<DocumentId, std::vector<DocumentId>> graph;
  for (const auto& [docId, description] : catalogues) {
    const auto& candidate = resolveNode(documents, description);
    std::set<DocumentId> dependencies;
    for (const auto& reference :
         services.referenceDescriptionProvider->createDescriptions(
             documentOf(candidate))) {
      if (reference.local || !reference.isResolved()) continue;
      const auto targetId = reference.targetDocumentId;
      if (targetId != docId && catalogues.contains(targetId))
        dependencies.insert(targetId);
    }
    graph.emplace(docId, std::vector<DocumentId>(dependencies.begin(),
                                                 dependencies.end()));
  }

  std::map<DocumentId, std::vector<AstNodeDescription>> cycles;
  for (auto& component : computeStronglyConnectedComponents(graph)) {
    if (component.size() < 2) continue;
    for (const auto id : component) {
      auto& participants = cycles[id];
      for (const auto other : component)
        if (other != id) participants.push_back(catalogues.at(other));
    }
  }
  return cycles;
}

void XsmpcatValidator::checkService(const Service& service,
                                    Acceptor& accept) const {
  if (service.name.empty() || isBuiltinUri(documentOf(service).uri)) {
    return;
  }
  // Workspace-wide, unfiltered by project visibility.
  const auto workspace = workspaceFacts();
  const auto same = workspace->servicesByName.find(service.name);
  if (same == workspace->servicesByName.end()) return;
  const auto& documents = *services.shared.documents;
  std::vector<cursiv::DiagnosticRelatedInformation> related;
  for (const auto& description : same->second) {
    const auto& candidate = resolveNode(documents, description);
    if (&candidate == &service) continue;
    const auto [begin, end] = rangeForFeature(candidate, "name", 0);
    related.push_back({.uri = std::string(documentOf(candidate).uri),
                       .message = std::string(description.name),
                       .range = {.begin = begin, .end = end}});
  }
  if (!related.empty()) {
    accept.error(service, "Duplicated Service name.")
        .property<&NamedElement::name>()
        .relatedInformation(
            std::span<const cursiv::DiagnosticRelatedInformation>(related));
  }
}

void XsmpcatValidator::checkNamedElementWithMultiplicity(
    const NamedElementWithMultiplicity& element, Acceptor& accept) const {
  if (element.multiplicity == nullptr) {
    return;
  }
  // `getLower` and `getUpper` are silent: what a bound fails on is reported
  // here.
  (void)getValueAs(element.multiplicity->lower, PTK::Int64, &accept);
  (void)getValueAs(element.multiplicity->upper, PTK::Int64, &accept);
  const auto lower = getLower(element);
  if (lower && *lower < 0) {
    accept
        .error(*element.multiplicity,
               "Lower bound shall be a positive number or 0.")
        .property<&Multiplicity::lower>();
  }

  const auto upper = getUpper(element);
  // An upper bound of -1 means no limit.
  if (lower && upper && *upper != -1 && *upper < *lower) {
    accept
        .error(*element.multiplicity,
               "Lower bound shall be less or equal to the upper bound, if "
               "present.\nUpper bound shall be -1 or larger or equal to the "
               "lower bound.")
        .property<&Multiplicity::lower>();
  }
}

void XsmpcatValidator::checkOperation(const Operation& operation,
                                      Acceptor& accept) const {
  std::unordered_set<const Type*> raised;
  for (std::size_t i = 0; i < operation.raisedException.size(); ++i) {
    const auto* exception = operation.raisedException[i].get();
    if (exception == nullptr) {
      continue;
    }
    checkTypeReferenceExtras<&Operation::raisedException>(operation, *exception,
                                                          accept, i);
    if (!raised.insert(exception).second) {
      accept.error(operation, "Duplicated exception.")
          .property<&Operation::raisedException>(i)
          .data(diagnosticData(DuplicatedException));
    }
  }

  std::unordered_set<std::string_view> parameterNames;
  if (operation.returnParameter != nullptr
      && !operation.returnParameter->name.empty()) {
    parameterNames.insert(operation.returnParameter->name);
  }
  auto requireDefaultValue = false;
  for (const auto* parameter : operation.parameter) {
    if (parameter == nullptr) {
      continue;
    }
    if (!parameter->name.empty()
        && !parameterNames.insert(parameter->name).second) {
      accept.error(*parameter, "Duplicated parameter name.")
          .property<&NamedElement::name>();
    }
    // Defaults come last, as in C++: after one parameter with a default, every
    // parameter needs one.
    if (parameter->default_ != nullptr) {
      requireDefaultValue = true;
    } else if (requireDefaultValue) {
      accept.error(*parameter, "The Parameter requires a default value.")
          .data(diagnosticData(MissingValue));
    }
  }

  const auto* isStatic = findAttribute(operation, "Attributes.Static");
  if (attributeBoolValueOf(isStatic).value_or(false)) {
    if (cursiv::ast_cast<const Interface>(operation.parent) != nullptr) {
      accept
          .error(*isStatic, "An Operation of an Interface shall not be static.")
          .data(diagnosticData(InvalidAttribute));
    }
    if (const auto* isVirtual = findAttribute(operation, "Attributes.Virtual");
        attributeBoolValueOf(isVirtual).value_or(false)) {
      accept
          .error(*isVirtual,
                 "An Operation shall not be both Static and Virtual.")
          .data(diagnosticData(InvalidAttribute));
    }
    if (const auto* isAbstract =
            findAttribute(operation, "Attributes.Abstract");
        attributeBoolValueOf(isAbstract).value_or(false)) {
      accept
          .error(*isAbstract,
                 "An Operation shall not be both Static and Abstract.")
          .data(diagnosticData(InvalidAttribute));
    }
  }

  if (const auto* isConstructor =
          findAttribute(operation, "Attributes.Constructor");
      attributeBoolValueOf(isConstructor).value_or(false)) {
    if (attributeBoolValueOf(isStatic).value_or(false)) {
      accept.error(*isStatic, "A Constructor shall not be Static.")
          .data(diagnosticData(InvalidAttribute));
    }
    if (const auto* isVirtual = findAttribute(operation, "Attributes.Virtual");
        attributeBoolValueOf(isVirtual).value_or(false)) {
      accept.error(*isVirtual, "A Constructor shall not be Virtual.")
          .data(diagnosticData(InvalidAttribute));
    }
    if (const auto* isConst = findAttribute(operation, "Attributes.Const");
        attributeBoolValueOf(isConst).value_or(false)) {
      accept.warning(*isConst, "Const attribute is ignored for a Constructor.")
          .data(diagnosticData(InvalidAttribute));
    }
    if (operation.returnParameter != nullptr) {
      accept
          .error(operation,
                 "A Constructor shall not have any return parameters.")
          .property<&Operation::returnParameter>();
    }
  }
}

void XsmpcatValidator::checkParameter(const Parameter& parameter,
                                      Acceptor& accept) const {
  if (const auto* type = parameter.type.get(); type != nullptr) {
    checkTypeReferenceExtras<&Parameter::type>(parameter, *type, accept);
    if (parameter.default_
        && (isByPointer(parameter) || isA<ValueReference>(type))
        && !isNullptr(parameter.default_))
      accept.error(*parameter.default_,
                   "A pointer parameter default shall be nullptr.");
    else if (parameter.default_ && isByReference(parameter)
             && !isConst(parameter))
      accept.error(*parameter.default_,
                   "A non-const reference parameter cannot bind to a default "
                   "value expression.");
    else if (!isNullptr(parameter.default_))
      checkValueBounds(*type, parameter.default_, accept);
    // A const reference binds to the pointer that `nullptr` makes.
    else if (!isPointer(parameter)
             && !(isByReference(parameter) && isPointerType(type)))
      accept.error(*parameter.default_, nullptrNeedsAPointer);
  }
  if (const auto* isByPointer =
          findAttribute(parameter, "Attributes.ByPointer");
      attributeBoolValueOf(isByPointer).value_or(false)
      && attributeBoolValue(parameter, "Attributes.ByReference")
             .value_or(false)) {
    accept
        .error(*isByPointer,
               "A Parameter shall not be both ByPointer and ByReference.")
        .data(diagnosticData(InvalidAttribute));
  }
}

void XsmpcatValidator::checkReturnParameter(const ReturnParameter& parameter,
                                            Acceptor& accept) const {
  // Not a NamedElement, but its name reaches the generated C++ too.
  if (isCppKeyword(parameter.name)) {
    accept.error(parameter, "A name shall not be an ISO/ANSI C++ keyword.")
        .property<&ReturnParameter::name>();
  }
  if (const auto* type = parameter.type.get(); type != nullptr) {
    checkTypeReferenceExtras<&ReturnParameter::type>(parameter, *type, accept);
  }
  if (const auto* isByPointer =
          findAttribute(parameter, "Attributes.ByPointer");
      attributeBoolValueOf(isByPointer).value_or(false)
      && attributeBoolValue(parameter, "Attributes.ByReference")
             .value_or(false)) {
    accept
        .error(*isByPointer,
               "A Parameter shall not be both ByPointer and ByReference.")
        .data(diagnosticData(InvalidAttribute));
  }
  checkAttributeUsages(parameter, accept);
}

void XsmpcatValidator::checkEntryPoint(const EntryPoint& entryPoint,
                                       Acceptor& accept) const {
  for (std::size_t i = 0; i < entryPoint.input.size(); ++i) {
    const auto* field = entryPoint.input[i].get();
    if (field == nullptr) {
      continue;
    }
    checkFieldReferenceExtras<&EntryPoint::input>(entryPoint, *field, accept,
                                                  i);
    if (std::ranges::none_of(field->modifiers, isInputModifier)) {
      accept.error(entryPoint, "Field is not an Input.")
          .property<&EntryPoint::input>(i);
    }
  }
  for (std::size_t i = 0; i < entryPoint.output.size(); ++i) {
    const auto* field = entryPoint.output[i].get();
    if (field == nullptr) {
      continue;
    }
    checkFieldReferenceExtras<&EntryPoint::output>(entryPoint, *field, accept,
                                                   i);
    if (std::ranges::none_of(field->modifiers, isOutputModifier)) {
      accept.error(entryPoint, "Field is not an Output.")
          .property<&EntryPoint::output>(i);
    }
  }
}

}  // namespace xsmp
