#include <algorithm>
#include <cmath>
#include <set>
#include <utility>
#include <xsmp/DeploymentSemantics.hpp>
#include <xsmp/SharedCoreServices.hpp>
#include <xsmp/solver/Solver.hpp>
#include <xsmp/utils/TreeUtils.hpp>
#include <xsmp/utils/XsmpUtils.hpp>
#include <xsmp/validation/IssueCodes.hpp>

namespace xsmp {

bool hasModifier(const VisibilityElement& node, std::string_view text) {
  return std::ranges::find(node.modifiers, text) != node.modifiers.end();
}

std::vector<const Field*> valueFields(const AstNode* type) {
  const auto* structure = cursiv::ast_cast<Structure>(type);
  return structure ? getAllFields(*structure) : std::vector<const Field*>{};
}

bool namesReturnParameter(const Operation& operation, std::string_view name) {
  const auto* result = operation.returnParameter;
  return result && name == (result->name.empty() ? "return" : result->name);
}

std::optional<std::int64_t> arraySize(const Array& array) {
  const auto size = getValueAs(array.size, PTK::Int64);
  const auto count = size ? integralValueOf(*size) : std::nullopt;
  return count && *count >= 0 ? count : std::nullopt;
}

namespace {

using Wide = __int128;

// The value of an integer literal. A literal beyond 2^120 reads as 2^120, which
// no integral type holds.
Wide parseInteger(std::string_view text) {
  const auto negative = text.starts_with('-');
  if (negative || text.starts_with('+')) text.remove_prefix(1);
  Wide result = 0;
  for (const auto digit : text) {
    if (result > (Wide{1} << 120)) break;
    result = result * 10 + (digit - '0');
  }
  return negative ? -result : result;
}

// The value of an integral literal, or nothing for any other value.
std::optional<Wide> integralOf(const Value* value) {
  if (const auto* number = cursiv::ast_cast<IntValue>(value))
    return parseInteger(number->value);
#define XSMP_INTEGRAL_VALUE(T)                         \
  if (const auto* number = cursiv::ast_cast<T>(value)) \
  return Wide{number->value}
  XSMP_INTEGRAL_VALUE(Int8Value);
  XSMP_INTEGRAL_VALUE(Int16Value);
  XSMP_INTEGRAL_VALUE(Int32Value);
  XSMP_INTEGRAL_VALUE(Int64Value);
  XSMP_INTEGRAL_VALUE(UInt8Value);
  XSMP_INTEGRAL_VALUE(UInt16Value);
  XSMP_INTEGRAL_VALUE(UInt32Value);
  XSMP_INTEGRAL_VALUE(UInt64Value);
#undef XSMP_INTEGRAL_VALUE
  return std::nullopt;
}

// The value of a floating literal, or nothing for any other value.
std::optional<double> floatOf(const Value* value) {
  if (const auto* number = cursiv::ast_cast<Float32Value>(value))
    return number->value;
  if (const auto* number = cursiv::ast_cast<Float64Value>(value))
    return number->value;
  if (const auto* number = cursiv::ast_cast<FloatValue>(value))
    return number->value;
  return std::nullopt;
}

// The field called `name` that a structure value of `type` can set, whatever
// its visibility. A type comes before its bases.
const Field* namedField(const Type* type, std::string_view name) {
  const Field* found = nullptr;
  members(type, [&](const AstNode* node) {
    const auto* field = cursiv::ast_cast<Field>(node);
    if (!field || field->name != name) return true;
    found = field;
    return false;
  });
  return found;
}

}  // namespace

void checkValue(const Value* value, const Type* type,
                cursiv::ValidationAcceptor& accept) {
  if (!value || !type) return;
  const auto mismatch = [&] {
    accept
        .error(*value, "The value shall be compatible with type "
                           + qualifiedName(*type) + ".")
        .code("xsmpcfg.value.type")
        .data(diagnosticData("xsmpcfg.value.type"));
  };

  if (const auto* array = cursiv::ast_cast<Array>(type)) {
    const auto* values = cursiv::ast_cast<ArrayValue>(value);
    if (!values) {
      mismatch();
      return;
    }

    const auto size = arraySize(*array);
    const auto start = values->startIndex.value_or(0);
    if (size
        && start + static_cast<std::int64_t>(values->elements.size()) > *size) {
      const auto limit = std::to_string(*size);
      auto diagnostic = accept.error(
          *values, start == 0 ? "The array value shall not contain more than "
                                    + limit + " item(s)."
                              : "The array value shall not exceed " + limit
                                    + " item(s) when StartIndex is applied.");
      if (!cursiv::container_of_type<Assembly>(values))
        diagnostic.code("xsmpcfg.array.size")
            .data(diagnosticData("xsmpcfg.array.size"));
    }

    // Items beyond the size are reported above: only those that fit are read.
    for (std::size_t index = 0;
         index < values->elements.size()
         && (!size || static_cast<std::int64_t>(index) < *size);
         ++index)
      checkValue(values->elements[index], array->itemType.get(), accept);
    return;
  }

  if (cursiv::ast_cast<Structure>(type)) {
    const auto* values = cursiv::ast_cast<StructureValue>(value);
    if (!values) {
      mismatch();
      return;
    }

    // Each element names its field, or takes the next field not yet used.
    const auto fields = valueFields(type);
    std::unordered_set<const Field*> used;
    std::size_t next = 0;
    for (const auto* child : values->elements) {
      const Field* field = nullptr;
      // A designated element holds its value; a positional one is its own
      // value.
      const auto* initializer = child;
      auto unsafe = false;
      std::string_view named;
      if (const auto* designated =
              cursiv::ast_cast<CfgStructureFieldValue>(child)) {
        unsafe = designated->field && designated->field->unsafe;
        named = designated->field ? designated->field->reference.refText
                                  : std::string_view{};
        if (designated->field) initializer = designated->value;
      } else if (const auto* designated = cursiv::ast_cast<FieldValue>(child)) {
        if (designated->field) {
          unsafe = designated->field->unsafe;
          field = cursiv::ast_cast<Field>(pathTarget(designated->field));
          if (field)
            named = field->name;
          else {
            if (!unsafe)
              accept.error(*designated->field, "The structure field path '"
                                                   + pathText(designated->field)
                                                   + "' does not exist on type "
                                                   + qualifiedName(*type)
                                                   + ".");
            continue;
          }
        }
        initializer = designated->value;
      }

      if (!named.empty()) {
        if (!field) field = namedField(type, named);
        if (!field) {
          if (!unsafe)
            accept
                .error(*child, "The structure field '" + std::string(named)
                                   + "' does not exist on type "
                                   + qualifiedName(*type) + ".")
                .range(rangeForFeature(*child, "field"));
          continue;
        }
        if (used.contains(field)
            && cursiv::ast_cast<CfgStructureFieldValue>(child)) {
          if (!unsafe)
            accept
                .error(*child,
                       "The structure field '" + std::string(named)
                           + "' shall not be initialized more than once.")
                .range(rangeForFeature(*child, "field"));
          continue;
        }
      } else {
        while (next < fields.size() && used.contains(fields[next])) ++next;
        if (next == fields.size()) {
          accept.error(*child,
                       "The structure value shall not contain more values than "
                       "the fields of "
                           + qualifiedName(*type) + ".");
          continue;
        }
        field = fields[next++];
      }

      used.insert(field);
      if (!unsafe) checkValue(initializer, field->type.get(), accept);
    }
    return;
  }

  // A type with no primitive kind has nothing to compare the value with.
  const auto kind = getPTK(type);
  auto matches = kind == PTK::None;
#define XSMP_VALUE(K, T)                  \
  case PTK::K:                            \
    matches = cursiv::ast_cast<T>(value); \
    break
  switch (kind) {
    XSMP_VALUE(Bool, BoolValue);
    XSMP_VALUE(Char8, Char8Value);
    XSMP_VALUE(String8, String8Value);
    XSMP_VALUE(DateTime, DateTimeValue);
    XSMP_VALUE(Duration, DurationValue);
    XSMP_VALUE(Enum, EnumerationValue);
    // A number without a suffix suits every size.
    case PTK::Float32:
      matches = cursiv::ast_cast<Float32Value>(value)
                || cursiv::ast_cast<FloatValue>(value);
      break;
    case PTK::Float64:
      matches = cursiv::ast_cast<Float64Value>(value)
                || cursiv::ast_cast<FloatValue>(value);
      break;
#define XSMP_INTEGRAL(K)                            \
  case PTK::K:                                      \
    matches = cursiv::ast_cast<K##Value>(value)     \
              || cursiv::ast_cast<IntValue>(value); \
    break
      XSMP_INTEGRAL(Int8);
      XSMP_INTEGRAL(Int16);
      XSMP_INTEGRAL(Int32);
      XSMP_INTEGRAL(Int64);
      XSMP_INTEGRAL(UInt8);
      XSMP_INTEGRAL(UInt16);
      XSMP_INTEGRAL(UInt32);
      XSMP_INTEGRAL(UInt64);
#undef XSMP_INTEGRAL
    default:
      break;
  }
#undef XSMP_VALUE

  if (!matches) {
    mismatch();
    return;
  }

  if (const auto* text = cursiv::ast_cast<DurationValue>(value);
      text && !isValidDuration(text->value) && !holdsReportedLiteral(*text))
    accept.error(*value, "The value shall be a valid Duration (e.g: PT1S).");
  if (const auto* text = cursiv::ast_cast<DateTimeValue>(value);
      text && !isValidDateTime(text->value) && !holdsReportedLiteral(*text))
    accept.error(
        *value,
        "The value shall be a valid DateTime (e.g: 1970-01-01T00:00:00Z).");

  if (const auto* written = cursiv::ast_cast<EnumerationValue>(value))
    if (const auto* enumeration = cursiv::ast_cast<Enumeration>(type)) {
      if (const auto* literal = written->reference.get()) {
        if (literal->parent != enumeration)
          accept.error(*value,
                       "Enumeration literal " + std::string(literal->name)
                           + " is not of type " + qualifiedName(*type) + ".");
      } else if (written->value
                 && std::ranges::none_of(
                     enumeration->literal, [&](const auto* literal) {
                       const auto number =
                           getValueAs(literal->value, PTK::Int32);
                       return number
                              && integralValueOf(*number) == written->value;
                     })) {
        accept.error(*value, std::to_string(*written->value)
                                 + " is not the value of a literal of type "
                                 + qualifiedName(*type) + ".");
      }
    }

  // The value as the solver holds it, for the bounds of its type.
  SolverValue typed{.type = kind};
  if (isIntegralType(kind)) {
    // A DateTime or a Duration has no number to check here.
    const auto raw = integralOf(value);
    if (!raw) return;
    const auto isSigned = isSignedIntegral(kind);
    const auto top =
        Wide{1} << (isSigned ? integralWidth(kind) - 1 : integralWidth(kind));
    if (*raw < (isSigned ? -top : Wide{0}) || *raw >= top) {
      accept.error(*value, "Conversion overflow for type "
                               + std::string(ptkName(kind)) + ".");
      return;
    }
    typed.integral = static_cast<std::uint64_t>(*raw);
  } else if (isFloatingType(kind)) {
    const auto raw = floatOf(value);
    if (!raw) return;
    if (!std::isfinite(*raw)
        || (kind == PTK::Float32 && !std::isfinite(static_cast<float>(*raw)))) {
      accept.error(*value, "Conversion overflow for type "
                               + std::string(ptkName(kind)) + ".");
      return;
    }
    typed.floating = kind == PTK::Float32 ? static_cast<float>(*raw) : *raw;
  } else if (const auto* text = cursiv::ast_cast<String8Value>(value)) {
    typed.text = text->value;
  }
  checkTypeBounds(typed, *type, *value, accept);
}

namespace {

// The type of the value a connection reads or writes at `path`, or null when it
// is unsafe, does not resolve, or names no value. Those are reported elsewhere.
const ValueType* connectedType(const Path* path) {
  return path && !path->unsafe
             ? cursiv::ast_cast<ValueType>(targetType(pathTarget(path)))
             : nullptr;
}

using TypePairs = std::set<std::pair<const Type*, const Type*>>;

// Strictly compatible or semantically equivalent, as ECSS-E-ST-40-07 Table 5-4
// says. `seen` stops at a recursive type, which is reported where it is
// declared.
bool equivalentTypes(const Type* a, const Type* b, TypePairs& seen) {
  if (a == b || !a || !b || !seen.emplace(a, b).second) return true;

  const auto* leftArray = cursiv::ast_cast<Array>(a);
  const auto* rightArray = cursiv::ast_cast<Array>(b);
  if (leftArray && rightArray) {
    const auto left = arraySize(*leftArray), right = arraySize(*rightArray);
    return (!left || !right || left == right)
           && equivalentTypes(leftArray->itemType.get(),
                              rightArray->itemType.get(), seen);
  }

  // Exactly a structure: a class is only equivalent to itself.
  if (a->type == &cursiv::ast_type_tag<Structure>
      && b->type == &cursiv::ast_type_tag<Structure>) {
    const auto left = valueFields(a), right = valueFields(b);
    return std::ranges::equal(left, right, [&](const auto* f, const auto* g) {
      return equivalentTypes(f->type.get(), g->type.get(), seen);
    });
  }

  const auto* leftString = cursiv::ast_cast<String>(a);
  const auto* rightString = cursiv::ast_cast<String>(b);
  if (leftString || rightString) {
    if (!leftString || !rightString) return false;
    const auto left = getValueAs(leftString->length, PTK::Int64);
    const auto right = getValueAs(rightString->length, PTK::Int64);
    return !left || !right || left->integral == right->integral;
  }

  const auto left = getPTK(a), right = getPTK(b);
  if (left == PTK::None || left == PTK::Enum) return false;
  const auto isTime = [](PTK kind) {
    return kind == PTK::Duration || kind == PTK::DateTime;
  };
  return left == right || (isTime(left) && right == PTK::Int64)
         || (left == PTK::Int64 && isTime(right));
}

}  // namespace

void checkFieldConnection(const AstNode& node, const Path* output,
                          const Path* input,
                          cursiv::ValidationAcceptor& accept) {
  TypePairs seen;
  if (!equivalentTypes(connectedType(output), connectedType(input), seen))
    accept.error(node,
                 "The Output and Input Fields shall have the same or a "
                 "semantically equivalent type.");
}

void checkEventConnection(const EventLink& link,
                          cursiv::ValidationAcceptor& accept) {
  if (!link.ownerPath || link.ownerPath->unsafe || !link.clientPath
      || link.clientPath->unsafe)
    return;
  const auto* source =
      cursiv::ast_cast<EventSource>(pathTarget(link.ownerPath));
  const auto* sink = cursiv::ast_cast<EventSink>(pathTarget(link.clientPath));
  if (source && sink && source->type.get() && sink->type.get()
      && source->type.get() != sink->type.get())
    accept.error(
        link,
        "The Event Source and the Event Sink shall have the same Event Type.");
}

// In both, an empty text is an optional value that was not written.
bool validDuration(std::string_view value) {
  return value.empty() || isValidDuration(value);
}

bool validDateTime(std::string_view value) {
  return value.empty() || isValidDateTime(value);
}

void checkDuration(const AstNode& node, std::string_view value,
                   std::string_view label, cursiv::ValidationAcceptor& accept) {
  if (!validDuration(value))
    accept.error(
        node, std::string(label) + " shall be a valid Duration (e.g: PT1S).");
}

void checkDate(const AstNode& node, std::string_view value,
               std::string_view label, cursiv::ValidationAcceptor& accept) {
  if (!validDateTime(value))
    accept.error(
        node, std::string(label)
                  + " shall be a valid DateTime (e.g: 1970-01-01T00:00:00Z).");
}

void checkOperationParameters(const AstNode& call, const Operation* operation,
                              const std::vector<ParameterValue*>& parameters,
                              cursiv::ValidationAcceptor& accept) {
  if (!operation) return;
  const auto& signature = operation->parameter;
  std::unordered_set<const Parameter*> given;
  std::ptrdiff_t next = 0;
  for (auto* value : parameters) {
    // The value expected back from the call, which may stand anywhere.
    if (namesReturnParameter(*operation, value->parameter)) {
      checkValue(value->value, operation->returnParameter->type.get(), accept);
      continue;
    }
    const auto found = std::ranges::find_if(signature, [&](const auto* other) {
      return other->name == value->parameter;
    });
    if (found == signature.end()) {
      accept.error(*value, "The parameter '" + std::string(value->parameter)
                               + "' shall resolve to a Parameter of operation "
                               + std::string(operation->name) + ".");
      continue;
    }
    checkValue(value->value, (*found)->type.get(), accept);
    // A repeated name is reported on its own.
    const auto index = found - signature.begin();
    if (given.insert(*found).second && index < next)
      accept.error(*value,
                   "The parameters shall come in the order of operation "
                       + std::string(operation->name) + ".");
    next = std::max(next, index + 1);
  }

  for (const auto* parameter : signature)
    if (!parameter->default_ && !given.contains(parameter))
      accept.error(call, "The parameter '" + std::string(parameter->name)
                             + "' of operation " + std::string(operation->name)
                             + " needs a value.");
}

}  // namespace xsmp
