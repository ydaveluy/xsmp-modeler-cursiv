#include <algorithm>
#include <cctype>
#include <cursiv/syntax-tree/AstNode.hpp>
#include <format>
#include <unordered_set>
#include <vector>
#include <xsmp/ast.hpp>
#include <xsmp/solver/Solver.hpp>
#include <xsmp/utils/TreeUtils.hpp>
#include <xsmp/utils/Utf8.hpp>
#include <xsmp/utils/XsmpUtils.hpp>

namespace xsmp {

bool isBaseOfClass(const Class* parent, const Type* base) {
  std::unordered_set<const Type*> visited;
  while (base != nullptr && visited.insert(base).second) {
    if (base == parent) {
      return true;
    }
    const auto* cls = cursiv::ast_cast<const Class>(base);
    if (cls == nullptr) {
      return false;
    }
    base = cls->base.has_value() ? cls->base->get() : nullptr;
  }
  return false;
}

bool isBaseOfComponent(const Component* parent, const Type* base) {
  std::unordered_set<const Type*> visited;
  while (base != nullptr && visited.insert(base).second) {
    if (base == parent) {
      return true;
    }
    const auto* component = cursiv::ast_cast<const Component>(base);
    if (component == nullptr) {
      return false;
    }
    base = component->base.has_value() ? component->base->get() : nullptr;
  }
  return false;
}

namespace {

bool isBaseOfInterfaceImpl(const Interface* parent, const Type* base,
                           std::unordered_set<const Type*>& visited) {
  if (base == nullptr || !visited.insert(base).second) {
    return false;
  }
  if (base == parent) {
    return true;
  }
  const auto* interface = cursiv::ast_cast<const Interface>(base);
  if (interface == nullptr) {
    return false;
  }
  for (const auto& parentBase : interface->base) {
    if (isBaseOfInterfaceImpl(parent, parentBase.get(), visited)) {
      return true;
    }
  }
  return false;
}

}  // namespace

bool isBaseOfInterface(const Interface* parent, const Type* base) {
  std::unordered_set<const Type*> visited;
  return isBaseOfInterfaceImpl(parent, base, visited);
}

namespace {

template <typename T, typename U>
bool isA(const U* node) {
  return cursiv::ast_cast<const T>(node) != nullptr;
}

template <typename T>
bool anyElementIsA(const std::vector<NamedElement*>& elements) {
  return std::any_of(elements.begin(), elements.end(), [](const auto* element) {
    return element != nullptr && isA<T>(element);
  });
}

bool isBaseOfReferenceTypeImpl(const Type* parent,
                               std::string_view parentQualifiedName,
                               const Type* base,
                               std::unordered_set<const Type*>& visited) {
  if (base == nullptr || !visited.insert(base).second) {
    return false;
  }
  if (base == parent) {
    return true;
  }
  if (const auto* interface = cursiv::ast_cast<const Interface>(base)) {
    for (const auto& parentBase : interface->base) {
      if (isBaseOfReferenceTypeImpl(parent, parentQualifiedName,
                                    parentBase.get(), visited)) {
        return true;
      }
    }
    return false;
  }
  const auto* component = cursiv::ast_cast<const Component>(base);
  if (component == nullptr) {
    return false;
  }

  // A component implements these interfaces implicitly, by its kind or by what
  // it declares. `Smp.IObject` and `Smp.IComponent` hold for any component.
  if (parentQualifiedName == "Smp.IObject"
      || parentQualifiedName == "Smp.IComponent"
      || (parentQualifiedName == "Smp.IModel" && isA<Model>(component))
      || (parentQualifiedName == "Smp.IService" && isA<Service>(component))
      || (parentQualifiedName == "Smp.IEntryPointPublisher"
          && anyElementIsA<EntryPoint>(component->elements))
      || (parentQualifiedName == "Smp.IComposite"
          && anyElementIsA<Container>(component->elements))
      || (parentQualifiedName == "Smp.IAggregate"
          && anyElementIsA<Reference>(component->elements))
      || (parentQualifiedName == "Smp.IEventConsumer"
          && anyElementIsA<EventSink>(component->elements))
      || (parentQualifiedName == "Smp.IEventProvider"
          && anyElementIsA<EventSource>(component->elements))
      || (parentQualifiedName == "Smp.IDynamicInvocation"
          && (anyElementIsA<Operation>(component->elements)
              || anyElementIsA<Property>(component->elements)))) {
    return true;
  }
  if (component->base.has_value()
      && isBaseOfReferenceTypeImpl(parent, parentQualifiedName,
                                   component->base->get(), visited)) {
    return true;
  }
  for (const auto& i : component->interface_) {
    if (isBaseOfReferenceTypeImpl(parent, parentQualifiedName, i.get(),
                                  visited)) {
      return true;
    }
  }
  return false;
}

}  // namespace

bool isBaseOfReferenceType(const Type* parent,
                           std::string_view parentQualifiedName,
                           const Type* base) {
  std::unordered_set<const Type*> visited;
  return isBaseOfReferenceTypeImpl(parent, parentQualifiedName, base, visited);
}

namespace {

bool isRecursiveTypeImpl(const Type* parent, const Type* other,
                         std::unordered_set<const Type*>& visited) {
  if (other == nullptr || !visited.insert(other).second) {
    return false;
  }
  if (other == parent) {
    return true;
  }
  if (const auto* array = cursiv::ast_cast<const Array>(other)) {
    return isRecursiveTypeImpl(parent, array->itemType.get(), visited);
  }
  if (const auto* structure = cursiv::ast_cast<const Structure>(other)) {
    for (const auto* element : structure->elements) {
      if (const auto* field = cursiv::ast_cast<const Field>(element)) {
        if (isRecursiveTypeImpl(parent, field->type.get(), visited)) {
          return true;
        }
      }
    }
    // A class holds what its base holds.
    if (const auto* clazz = cursiv::ast_cast<const Class>(other);
        clazz && clazz->base) {
      return isRecursiveTypeImpl(parent, clazz->base->get(), visited);
    }
  }
  return false;
}

}  // namespace

bool isRecursiveType(const Type* parent, const Type* other) {
  std::unordered_set<const Type*> visited;
  return isRecursiveTypeImpl(parent, other, visited);
}

bool isCppKeyword(std::string_view name) {
  static const std::unordered_set<std::string_view> names{
      "alignas",       "alignof",     "and",
      "and_eq",        "asm",         "auto",
      "bitand",        "bitor",       "bool",
      "break",         "case",        "catch",
      "char",          "char8_t",     "char16_t",
      "char32_t",      "class",       "compl",
      "concept",       "const",       "consteval",
      "constexpr",     "constinit",   "const_cast",
      "continue",      "co_await",    "co_return",
      "co_yield",      "decltype",    "default",
      "delete",        "do",          "double",
      "dynamic_cast",  "else",        "enum",
      "explicit",      "export",      "extern",
      "false",         "float",       "for",
      "friend",        "goto",        "if",
      "inline",        "int",         "long",
      "mutable",       "namespace",   "new",
      "noexcept",      "not",         "not_eq",
      "nullptr",       "operator",    "or",
      "or_eq",         "private",     "protected",
      "public",        "register",    "reinterpret_cast",
      "requires",      "return",      "short",
      "signed",        "sizeof",      "static",
      "static_assert", "static_cast", "struct",
      "switch",        "template",    "this",
      "thread_local",  "throw",       "true",
      "try",           "typedef",     "typeid",
      "typename",      "union",       "unsigned",
      "using",         "virtual",     "void",
      "volatile",      "wchar_t",     "while",
      "xor",           "xor_eq"};
  return names.contains(name);
}

bool isUUID(std::string_view text) noexcept {
  static constexpr std::size_t kGroupLengths[] = {8, 4, 4, 4, 12};
  std::size_t pos = 0;
  for (std::size_t group = 0; group < 5; ++group) {
    if (group > 0) {
      if (pos >= text.size() || text[pos] != '-') {
        return false;
      }
      ++pos;
    }
    const auto length = kGroupLengths[group];
    if (pos + length > text.size()) {
      return false;
    }
    for (std::size_t i = 0; i < length; ++i) {
      if (std::isxdigit(static_cast<unsigned char>(text[pos + i])) == 0) {
        return false;
      }
    }
    pos += length;
  }
  return pos == text.size();
}

std::optional<std::int64_t> getLower(
    const NamedElementWithMultiplicity& element) {
  if (element.optional_) {
    return 0;
  }
  const auto* multiplicity = element.multiplicity;
  if (multiplicity == nullptr) {
    return 1;
  }
  if (multiplicity->lower == nullptr && multiplicity->upper == nullptr) {
    return multiplicity->aux ? 1 : 0;
  }
  if (const auto value = getValueAs(multiplicity->lower, PTK::Int64)) {
    return integralValueOf(*value);
  }
  return std::nullopt;
}

std::optional<std::int64_t> getUpper(
    const NamedElementWithMultiplicity& element) {
  const auto* multiplicity = element.multiplicity;
  if (element.optional_ || multiplicity == nullptr) {
    return 1;
  }
  if (multiplicity->lower == nullptr && multiplicity->upper == nullptr) {
    return -1;
  }
  if (multiplicity->upper == nullptr && multiplicity->aux) {
    return -1;
  }
  // A single bound `[n]` gives both.
  const auto* bound =
      multiplicity->upper ? multiplicity->upper : multiplicity->lower;
  if (const auto value = getValueAs(bound, PTK::Int64)) {
    return integralValueOf(*value);
  }
  return std::nullopt;
}

Visibility getRealVisibility(const NamedElement& node) {
  const auto* visibilityElement =
      cursiv::ast_cast<const VisibilityElement>(&node);
  if (visibilityElement == nullptr) {
    return Visibility::Public;
  }
  // Exact-type check (not subtype): a Class/Exception container has its own
  // visibility semantics and does NOT get this "always public" treatment,
  // even though Class derives from Structure in the AST hierarchy.
  if (const auto* container = node.parent; container != nullptr) {
    if (container->type == &cursiv::ast_type_tag<Structure>
        || container->type == &cursiv::ast_type_tag<Interface>) {
      return Visibility::Public;
    }
  }
  const auto& modifiers = visibilityElement->modifiers;
  if (std::ranges::find(modifiers, "private") != modifiers.end()) {
    return Visibility::Private;
  }
  if (std::ranges::find(modifiers, "protected") != modifiers.end()) {
    return Visibility::Protected;
  }
  if (std::ranges::find(modifiers, "public") != modifiers.end()) {
    return Visibility::Public;
  }
  return Visibility::Private;  // no visibility modifier at all.
}

namespace {

// The nearest ancestor of `node` (inclusive) that is a `Type`, or nullptr.
const Type* nearestType(const cursiv::AstNode* node) {
  for (const auto* current = node; current != nullptr;
       current = current->parent) {
    if (const auto* type = cursiv::ast_cast<const Type>(current)) {
      return type;
    }
  }
  return nullptr;
}

// The nearest ancestor of `node` (inclusive) that is a `Namespace`, or
// nullptr.
const Namespace* nearestNamespace(const cursiv::AstNode* node) {
  for (const auto* current = node; current != nullptr;
       current = current->parent) {
    if (const auto* ns = cursiv::ast_cast<const Namespace>(current)) {
      return ns;
    }
  }
  return nullptr;
}

// True when `ancestor` is `node` itself or one of its (transitive) containers.
bool isAncestorOf(const cursiv::AstNode* ancestor,
                  const cursiv::AstNode* node) {
  for (const auto* current = node; current != nullptr;
       current = current->parent) {
    if (current == ancestor) {
      return true;
    }
  }
  return false;
}

}  // namespace

bool isEffectivelyPublic(const NamedElement& node) {
  return getRealVisibility(node) == Visibility::Public;
}

bool isEffectivelyPrivate(const NamedElement& node) {
  return getRealVisibility(node) == Visibility::Private;
}

bool isConstantVisibleFrom(const Expression& from, const Constant& element) {
  const auto* reader = nearestType(&from);
  const auto* owner = cursiv::ast_cast<const Type>(element.parent);
  if (reader == owner) {
    return true;
  }
  const auto visibility = getRealVisibility(element);
  return visibility == Visibility::Public
         || (visibility == Visibility::Protected
             && (isBaseOfClass(cursiv::ast_cast<const Class>(owner), reader)
                 || isBaseOfComponent(cursiv::ast_cast<const Component>(owner),
                                      reader)));
}

bool isTypeVisibleFrom(const cursiv::AstNode& from, const Type& element) {
  const auto visibility = getRealVisibility(element);
  if (visibility == Visibility::Protected
      && &documentOf(element) != &documentOf(from)) {
    return false;
  }
  if (visibility == Visibility::Private
      && !isAncestorOf(nearestNamespace(&element), &from)) {
    return false;
  }
  return true;
}

std::optional<bool> attributeBoolValueOf(const Attribute* attribute) {
  if (attribute == nullptr) {
    return std::nullopt;
  }
  const Expression* expr = attribute->value;
  if (expr == nullptr) {
    const auto* type =
        cursiv::ast_cast<const AttributeType>(attribute->type.get());
    expr = type != nullptr ? type->default_ : nullptr;
  }
  const auto value = getValueAs(expr, PTK::Bool);
  if (!value) {
    return std::nullopt;
  }
  return value->boolean;
}

bool isConstructorAttribute(const NamedElement& element) {
  return attributeBoolValue(element, "Attributes.Constructor").value_or(false);
}

bool isStaticAttribute(const NamedElement& element) {
  return !isConstructorAttribute(element)
         && attributeBoolValue(element, "Attributes.Static").value_or(false);
}

bool isAbstractAttribute(const NamedElement& element) {
  return !isStaticAttribute(element)
         && attributeBoolValue(element, "Attributes.Abstract").value_or(false);
}

bool isSimpleArrayAttribute(const NamedElement& element) {
  return attributeBoolValue(element, "Attributes.SimpleArray").value_or(false);
}

bool isString8(const Type* type) {
  return type != nullptr && hasQualifiedName(*type, "Smp.String8");
}

namespace {

// How a parameter or a return value is passed when no attribute decides.
enum class ArgKind : std::uint8_t { ByValue, ByPtr, ByRef };

ArgKind argKind(const Parameter& parameter) {
  const auto* type = parameter.type.get();
  const auto input = parameter.direction.empty() || parameter.direction == "in";
  if (cursiv::ast_cast<const ReferenceType>(type) != nullptr) {
    return input ? ArgKind::ByRef : ArgKind::ByPtr;
  }
  if ((cursiv::ast_cast<const NativeType>(type) != nullptr
       || cursiv::ast_cast<const ValueType>(type) != nullptr)
      && !input) {
    return ArgKind::ByPtr;
  }
  return ArgKind::ByValue;
}

ArgKind argKind(const ReturnParameter& parameter) {
  return cursiv::ast_cast<const ReferenceType>(parameter.type.get()) != nullptr
             ? ArgKind::ByPtr
             : ArgKind::ByValue;
}

// The by-pointer and by-reference rules of Parameter and ReturnParameter only
// differ in how the argument kind is derived.
template <class T>
bool byPointer(const T& element) {
  if (const auto value = attributeBoolValue(element, "Attributes.ByPointer")) {
    return *value;
  }
  return argKind(element) == ArgKind::ByPtr
         && !attributeBoolValue(element, "Attributes.ByReference")
                 .value_or(false);
}

template <class T>
bool byReference(const T& element) {
  if (const auto value =
          attributeBoolValue(element, "Attributes.ByReference")) {
    return *value;
  }
  return argKind(element) == ArgKind::ByRef
         && !attributeBoolValue(element, "Attributes.ByPointer")
                 .value_or(false);
}

template <class T>
bool constFlag(const T& element) {
  return attributeBoolValue(element, "Attributes.Const").value_or(false);
}

}  // namespace

bool isConst(const Parameter& element) {
  if (attributeBoolValue(element, "Attributes.Constructor").value_or(false)) {
    return false;
  }
  if (const auto value = attributeBoolValue(element, "Attributes.Const")) {
    return *value;
  }
  const auto input = element.direction.empty() || element.direction == "in";
  return input
         && cursiv::ast_cast<const ValueType>(element.type.get()) == nullptr;
}

bool isConst(const ReturnParameter& element) {
  return !attributeBoolValue(element, "Attributes.Constructor").value_or(false)
         && constFlag(element);
}

bool isConst(const Association& element) { return constFlag(element); }

bool isConst(const Property& element) { return constFlag(element); }

bool isConst(const Operation& element) {
  return !isConstructorAttribute(element) && constFlag(element);
}

bool isByPointer(const Parameter& element) { return byPointer(element); }

bool isByPointer(const ReturnParameter& element) { return byPointer(element); }

bool isByPointer(const Association& element) {
  if (const auto value = attributeBoolValue(element, "Attributes.ByPointer")) {
    return *value;
  }
  return cursiv::ast_cast<const ReferenceType>(element.type.get()) != nullptr;
}

bool isByPointer(const Property& element) {
  if (const auto value = attributeBoolValue(element, "Attributes.ByPointer")) {
    return *value;
  }
  return cursiv::ast_cast<const ReferenceType>(element.type.get()) != nullptr
         && !isByReference(element);
}

bool isByReference(const Parameter& element) { return byReference(element); }

bool isByReference(const ReturnParameter& element) {
  return byReference(element);
}

bool isByReference(const Property& element) {
  return attributeBoolValue(element, "Attributes.ByReference").value_or(false);
}

bool isPointerType(const Type* type) {
  return cursiv::ast_cast<const ValueReference>(type)
         || cursiv::ast_cast<const NativeType>(type);
}

bool isPointer(const Parameter& parameter) {
  return !isByReference(parameter)
         && (isByPointer(parameter) || isPointerType(parameter.type.get()));
}

namespace {

// The letter of the short escape of `c`, if it has one.
constexpr char escapeLetter(char c) {
  switch (c) {
    case '\a':
      return 'a';
    case '\b':
      return 'b';
    case '\f':
      return 'f';
    case '\n':
      return 'n';
    case '\r':
      return 'r';
    case '\t':
      return 't';
    case '\v':
      return 'v';
    default:
      return 0;
  }
}

}  // namespace

std::string cppEscaped(std::string_view text, char delimiter) {
  std::string result;
  result.reserve(text.size());
  for (std::size_t i = 0; i < text.size(); ++i) {
    const auto c = text[i];
    const auto byte = static_cast<unsigned char>(c);
    const auto rest = text.substr(i);
    const auto size = utf8CharacterSize(rest);
    if (c == '\\' || c == delimiter) {
      result += '\\';
      result += c;
    } else if (const auto letter = escapeLetter(c)) {
      result += '\\';
      result += letter;
    } else if (c == '\0'
               && !(rest.size() > 1
                    && std::isdigit(static_cast<unsigned char>(rest[1])))) {
      result += "\\0";
    } else if (size == 0 || byte < 0x20 || byte == 0x7F) {
      result += std::format("\\{:03o}", byte);
    } else if (rest.starts_with("\xEF\xBF\xBE")
               || rest.starts_with("\xEF\xBF\xBF")) {
      // U+FFFE and U+FFFF are no characters: raw, they would not show.
      result += rest[2] == '\xBE' ? "\\uFFFE" : "\\uFFFF";
      i += 2;
    } else {
      result += rest.substr(0, size);
      i += size - 1;
    }
  }
  return result;
}

std::string getSignature(const NamedElement& member) {
  const auto* operation = cursiv::ast_cast<const Operation>(&member);
  if (operation == nullptr) return std::string(member.name);
  auto signature = std::string(member.name) + "(";
  std::string_view separator;
  for (const auto* parameter : operation->parameter) {
    signature += separator;
    separator = ", ";
    const auto* type = parameter->type.get();
    // Attributes win over the default of the direction, so writing a default
    // out does not make a new overload.
    if (isConst(*parameter)) signature += "const ";
    signature +=
        type ? qualifiedName(*type) : std::string(parameter->type.refText);
    if (isByPointer(*parameter)) signature += '*';
    if (isByReference(*parameter)) signature += '&';
  }
  return signature + ")";
}

std::vector<const Field*> getAllFields(
    const Structure& structure, std::unordered_set<const Structure*>& visited) {
  std::vector<const Field*> result;
  if (!visited.insert(&structure).second) {
    return result;
  }
  if (const auto* clazz = cursiv::ast_cast<const Class>(&structure)) {
    if (clazz->base.has_value()) {
      if (const auto* base =
              cursiv::ast_cast<const Structure>(clazz->base->get())) {
        result = getAllFields(*base, visited);
      }
    }
  }
  for (const auto* element : structure.elements) {
    if (const auto* field = cursiv::ast_cast<const Field>(element);
        field != nullptr && !isStaticAttribute(*field)
        && isEffectivelyPublic(*field)) {
      result.push_back(field);
    }
  }
  return result;
}

std::vector<const Field*> getAllFields(const Structure& structure) {
  std::unordered_set<const Structure*> visited;
  return getAllFields(structure, visited);
}

}  // namespace xsmp
