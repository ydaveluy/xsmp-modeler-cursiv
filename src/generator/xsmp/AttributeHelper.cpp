#include <algorithm>
#include <iterator>
#include <xsmp/AttributeHelper.hpp>
#include <xsmp/solver/Solver.hpp>
#include <xsmp/utils/XsmpUtils.hpp>

namespace xsmp {

namespace {

// The value of a boolean attribute, false when it is absent.
template <class T>
bool flag(const T& element, std::string_view id) {
  return attributeBoolValue(element, id).value_or(false);
}

bool hasModifier(const VisibilityElement& element, std::string_view modifier) {
  return std::ranges::find(element.modifiers, modifier)
         != element.modifiers.end();
}

}  // namespace

bool isMutable(const NamedElement& element) {
  return flag(element, "Attributes.Mutable");
}

std::optional<std::string_view> getViewKind(const NamedElement& element) {
  // Indexed by the integer value of the `Smp.ViewKind` literal.
  static constexpr std::string_view names[] = {"None", "Debug", "Expert",
                                               "All"};

  const auto* attribute = findAttribute(element, "Attributes.View");
  const auto value =
      getValue(attribute != nullptr ? attribute->value : nullptr);
  if (!value) {
    return std::nullopt;
  }

  if (value->type == PTK::Enum) {
    if (value->literal == nullptr) {
      return std::nullopt;
    }
    auto literal = value->literal->name;
    if (!literal.starts_with("VK_")) {
      return std::nullopt;
    }
    literal.remove_prefix(3);
    const auto* name = std::ranges::find(names, literal);
    return name == std::end(names) ? std::nullopt
                                   : std::optional<std::string_view>(*name);
  }

  const auto integral = integralValueOf(*value);
  if (!integral || *integral < 0
      || *integral >= static_cast<std::int64_t>(std::size(names))) {
    return std::nullopt;
  }
  return names[*integral];
}

std::string_view operatorLiteral(const Operation& op) {
  const auto* attr = findAttribute(op, "Attributes.Operator");
  if (attr == nullptr || attr->value == nullptr) {
    return {};
  }
  const auto* attrType =
      cursiv::ast_cast<const AttributeType>(attr->type.get());
  if (attrType == nullptr || attrType->type.get() == nullptr) {
    return {};
  }
  const auto value = getValueAs(attr->value, *attrType->type.get());
  if (!value || value->type != PTK::Enum || value->literal == nullptr) {
    return {};
  }
  return value->literal->name;
}

bool isInput(const VisibilityElement& element) {
  return hasModifier(element, "input");
}

bool isOutput(const VisibilityElement& element) {
  return hasModifier(element, "output");
}

bool isState(const VisibilityElement& element) {
  return !hasModifier(element, "transient");
}

bool isAbstractType(const VisibilityElement& element) {
  return hasModifier(element, "abstract");
}

bool isAbstract(const NamedElement& element) {
  if (isStaticAttribute(element)) {
    return false;
  }
  if (cursiv::ast_cast<const Interface>(element.parent) != nullptr) {
    return true;
  }
  return flag(element, "Attributes.Abstract");
}

bool isVirtual(const NamedElement& element) {
  if (isConstructorAttribute(element)) {
    return false;
  }
  if (isAbstract(element)) {
    return true;
  }
  if (const auto v = attributeBoolValue(element, "Attributes.Virtual")) {
    return *v;
  }
  return cursiv::ast_cast<const ReferenceType>(element.parent) != nullptr
         && !isStaticAttribute(element);
}

std::optional<AccessKind> getAccessKind(const Property& element) {
  if (hasModifier(element, "readOnly")) {
    return AccessKind::ReadOnly;
  }
  if (hasModifier(element, "readWrite")) {
    return AccessKind::ReadWrite;
  }
  if (hasModifier(element, "writeOnly")) {
    return AccessKind::WriteOnly;
  }
  return std::nullopt;
}

bool isConstGetter(const Property& element) {
  return flag(element, "Attributes.ConstGetter");
}

}  // namespace xsmp
