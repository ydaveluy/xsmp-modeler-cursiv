#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cursiv/syntax-tree/AstNode.hpp>
#include <cursiv/validation/ValidationAcceptor.hpp>
#include <cursiv/workspace/Document.hpp>
#include <limits>
#include <numbers>
#include <regex>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <xsmp/LiteralConversions.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/solver/Solver.hpp>
#include <xsmp/utils/XsmpUtils.hpp>
#include <xsmp/validation/IssueCodes.hpp>

namespace xsmp {

std::string_view ptkName(PTK kind) noexcept {
  switch (kind) {
    case PTK::Bool:
      return "Bool";
    case PTK::Char8:
      return "Char8";
    case PTK::Float32:
      return "Float32";
    case PTK::Float64:
      return "Float64";
    case PTK::Int8:
      return "Int8";
    case PTK::Int16:
      return "Int16";
    case PTK::Int32:
      return "Int32";
    case PTK::Int64:
      return "Int64";
    case PTK::UInt8:
      return "UInt8";
    case PTK::UInt16:
      return "UInt16";
    case PTK::UInt32:
      return "UInt32";
    case PTK::UInt64:
      return "UInt64";
    case PTK::DateTime:
      return "DateTime";
    case PTK::Duration:
      return "Duration";
    case PTK::String8:
      return "String8";
    case PTK::Enum:
      return "Enum";
    case PTK::None:
      return "None";
  }
  return "None";
}

namespace {

using I128 = __int128;
using U128 = unsigned __int128;

std::string fqn(const cursiv::AstNode& element) {
  return qualifiedName(element);
}

// --- integral helpers --------------------------------------------------------
// `value` wrapped to the width of `type` and kept as a 64-bit pattern,
// sign-extended for a signed type.
SolverValue makeIntegral(I128 value, PTK type) {
  const auto width = integralWidth(type);
  auto bits = static_cast<std::uint64_t>(static_cast<U128>(value));
  if (isSignedIntegral(type)) {
    if (width < 64) {
      const std::uint64_t mask = (1ULL << width) - 1U;
      bits &= mask;
      if ((bits & (1ULL << (width - 1))) != 0U) {
        bits |= ~mask;  // sign-extend into the 64-bit storage
      }
    }
  } else if (width < 64) {
    bits &= (1ULL << width) - 1U;
  }
  SolverValue result;
  result.type = type;
  result.integral = bits;
  return result;
}

// The number an integral value stands for, read back from its bit pattern.
// 128 bits hold every Int64 and UInt64, so mixed operands compare correctly.
I128 trueValue(const SolverValue& value) {
  if (isSignedIntegral(value.type)) {
    return static_cast<I128>(static_cast<std::int64_t>(value.integral));
  }
  return static_cast<I128>(static_cast<U128>(value.integral));
}

SolverValue makeBool(bool value) {
  SolverValue result;
  result.type = PTK::Bool;
  result.boolean = value;
  return result;
}

SolverValue makeFloat(double value, PTK type) {
  SolverValue result;
  result.type = type;
  result.floating = (type == PTK::Float32)
                        ? static_cast<double>(static_cast<float>(value))
                        : value;
  return result;
}

SolverValue makeString(std::string value) {
  SolverValue result;
  result.type = PTK::String8;
  result.text = std::move(value);
  return result;
}

SolverValue makeChar(std::string value) {
  SolverValue result;
  result.type = PTK::Char8;
  result.text = std::move(value);
  return result;
}

SolverValue makeEnum(const EnumerationLiteral* literal) {
  SolverValue result;
  result.type = PTK::Enum;
  result.literal = literal;
  return result;
}

// --- conversions -------------------------------------------------------------
std::optional<SolverValue> toBool(const SolverValue& v) {
  switch (v.type) {
    case PTK::Bool:
      return v;
    default:
      if (isIntegralType(v.type)) {
        return makeBool(trueValue(v) != 0);
      }
      if (isFloatingType(v.type)) {
        return makeBool(v.floating != 0.0);
      }
      return std::nullopt;
  }
}

// An `xs:duration` with days, hours, minutes and seconds: `PT1S`, `P2DT1M`,
// `-PT0.5S`. Only the whole duration has a sign. The result is in nanoseconds,
// further fractional digits dropped; a field beyond 64 bits is not a duration.
std::optional<I128> parseDurationString(std::string_view text) {
  static const std::regex pattern(
      R"((-?)P(?:(\d+)D)?(T(?:(\d+)H)?(?:(\d+)M)?(?:(\d+)(?:\.(\d+))?S)?)?)");
  const std::string input(text);
  std::smatch match;

  // The pattern alone would also take a `T` with no time after it, or no field
  // at all.
  if (!std::regex_match(input, match, pattern)
      || (match[3].matched && match[3].str() == "T")
      || (!match[2].matched && !match[4].matched && !match[5].matched
          && !match[6].matched))
    return std::nullopt;

  auto fits = true;
  const auto field = [&fits](const std::ssub_match& group,
                             std::int64_t unit) -> I128 {
    if (!group.matched) return 0;
    const auto digits = group.str();
    std::int64_t value = 0;
    if (std::from_chars(digits.data(), digits.data() + digits.size(), value).ec
        != std::errc{})
      fits = false;
    return I128{value} * unit;
  };

  auto nanos = field(match[2], 86400) + field(match[4], 3600)
               + field(match[5], 60) + field(match[6], 1);
  nanos *= 1'000'000'000;
  if (match[7].matched) {
    auto digits = match[7].str();
    digits.resize(9, '0');
    nanos += std::stoll(digits);
  }

  if (!fits) return std::nullopt;
  return match[1].length() > 0 ? -nanos : nanos;
}

// An `xsd:dateTime`, in nanoseconds since 1970. Without a zone it reads as UTC,
// so that a value does not depend on the machine. Nine fractional digits are
// kept, further ones dropped.
std::optional<I128> parseDateTimeString(std::string_view text) {
  static const std::regex pattern(
      R"(^(-?[0-9]{4})-([0-9]{2})-([0-9]{2})(?:[Tt ]([0-9]{2}):([0-9]{2}):([0-9]{2})(?:[.,]([0-9]+))?)?(?:[Zz]|([+-])([0-9]{2}):([0-9]{2}))?$)");
  std::match_results<std::string_view::const_iterator> match;
  if (!std::regex_match(text.begin(), text.end(), match, pattern))
    return std::nullopt;
  const auto integer = [&](unsigned at) {
    return match[at].matched ? std::stoi(match[at].str()) : 0;
  };

  using namespace std::chrono;
  const year_month_day date{year{integer(1)},
                            month{static_cast<unsigned>(integer(2))},
                            day{static_cast<unsigned>(integer(3))}};
  const auto hour = integer(4), minute = integer(5), second = integer(6);
  const auto zoneHour = integer(9), zoneMinute = integer(10);

  auto fraction = match[7].str();
  fraction.resize(9, '0');
  const I128 nanos = std::stoll(fraction);

  // An `xsd:dateTime` allows 24:00:00 as the end of a day, and a zone up to
  // 14:00.
  if (!date.ok() || hour > 24 || minute > 59 || second > 59
      || (hour == 24 && (minute || second || nanos)) || zoneHour > 14
      || zoneMinute > 59 || (zoneHour == 14 && zoneMinute))
    return std::nullopt;

  const I128 zone = (zoneHour * 60 + zoneMinute) * 60;
  const auto seconds =
      static_cast<I128>(sys_days{date}.time_since_epoch().count()) * 86400
      + hour * 3600 + minute * 60 + second
      + (match[8].str() == "-" ? zone : -zone);
  return seconds * 1'000'000'000 + nanos;
}

// `source` as a value of `target`, or nothing when the type cannot hold it.
std::optional<SolverValue> fitIntegral(I128 source, PTK target) {
  auto result = makeIntegral(source, target);
  if (trueValue(result) != source) return std::nullopt;
  return result;
}

// `text` as a DateTime value, which counts from the SMP reference time.
std::optional<SolverValue> dateTimeValue(std::string_view text) {
  const auto ns = parseDateTimeString(text);
  return ns ? fitIntegral(*ns - I128{kDateTimeEpochSeconds} * 1'000'000'000,
                          PTK::DateTime)
            : std::nullopt;
}

std::optional<SolverValue> durationValue(std::string_view text) {
  const auto ns = parseDurationString(text);
  return ns ? fitIntegral(*ns, PTK::Duration) : std::nullopt;
}

// `v` as a value of `target`, or nothing when it cannot be one. A Bool reads as
// 0 or 1, and a String8 converts only to a DateTime or a Duration.
std::optional<SolverValue> toIntegral(const SolverValue& v, PTK target) {
  if (v.type == PTK::Bool) {
    return makeIntegral(v.boolean ? 1 : 0, target);
  }
  if (isIntegralType(v.type)) {
    if (v.type == target) {
      return v;
    }
    return fitIntegral(trueValue(v), target);
  }
  if (isFloatingType(v.type)) {
    // Only a whole number inside the 64-bit range converts.
    const auto d = v.floating;
    if (!std::isfinite(d) || std::trunc(d) != d || std::fabs(d) >= 0x1p64)
      return std::nullopt;
    return fitIntegral(static_cast<I128>(d), target);
  }
  if (v.type == PTK::String8 && target == PTK::DateTime)
    return dateTimeValue(v.text);
  if (v.type == PTK::String8 && target == PTK::Duration)
    return durationValue(v.text);
  return std::nullopt;
}

std::optional<SolverValue> toFloat(const SolverValue& v, PTK target) {
  if (v.type == PTK::Bool) {
    return makeFloat(v.boolean ? 1.0 : 0.0, target);
  }
  if (isIntegralType(v.type)) {
    return makeFloat(static_cast<double>(trueValue(v)), target);
  }
  if (isFloatingType(v.type)) {
    if (v.type == target) {
      return v;
    }
    return makeFloat(v.floating, target);
  }
  return std::nullopt;
}

// An integer reads as a Unicode code point, written in UTF-8.
std::optional<SolverValue> toCharValue(const SolverValue& v) {
  if (v.type == PTK::Char8) {
    return v;
  }
  if (isIntegralType(v.type)) {
    const auto cp = trueValue(v);
    if (cp < 0 || cp > 0x10FFFF) {
      return std::nullopt;
    }
    const auto code = static_cast<std::uint32_t>(cp);
    std::string utf8;
    if (code <= 0x7F) {
      utf8.push_back(static_cast<char>(code));
    } else if (code <= 0x7FF) {
      utf8.push_back(static_cast<char>(0xC0 | (code >> 6)));
      utf8.push_back(static_cast<char>(0x80 | (code & 0x3F)));
    } else if (code <= 0xFFFF) {
      utf8.push_back(static_cast<char>(0xE0 | (code >> 12)));
      utf8.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
      utf8.push_back(static_cast<char>(0x80 | (code & 0x3F)));
    } else {
      utf8.push_back(static_cast<char>(0xF0 | (code >> 18)));
      utf8.push_back(static_cast<char>(0x80 | ((code >> 12) & 0x3F)));
      utf8.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
      utf8.push_back(static_cast<char>(0x80 | (code & 0x3F)));
    }
    return makeChar(std::move(utf8));
  }
  return std::nullopt;
}

// --- integral literal parsing ------------------------------------------------
// The state of one evaluation: the Constants being evaluated, which is how a
// circular reference is caught, and the value of those already read, so that a
// chain of Constants that each read the previous one twice stays linear.
struct Ctx {
  std::unordered_set<const Constant*> evaluating;
  std::unordered_map<const Constant*, std::optional<SolverValue>> values;
  // A circular reference was met and not reported yet.
  bool circular = false;
};

// The value of an integer literal, in the first type that holds it, as in C++:
// Int32, then UInt32 unless it is decimal, then Int64 and UInt64. `u` rules out
// the signed types and `l` the 32-bit ones. `'` separates digits. A value
// beyond 64 bits is reported, and a digit outside its base gives nothing.
std::optional<SolverValue> parseIntegerLiteral(std::string_view raw,
                                               const IntegerLiteral& node,
                                               Acceptor* accept) {
  auto text = integer_value(raw);
  std::erase(text, '\'');
  if (text.empty()) {
    return std::nullopt;
  }

  // A time suffix (`5s`) now reads as nanoseconds with an `L` suffix, so only
  // the u/l suffixes remain: up to two, in any order.
  auto isUnsigned = false;
  auto isLong = false;
  for (auto i = 0; i < 2; ++i) {
    if (text.empty()) {
      break;
    }
    const auto c = text.back();
    if (c == 'u' || c == 'U') {
      isUnsigned = true;
      text.pop_back();
    } else if (c == 'l' || c == 'L') {
      isLong = true;
      text.pop_back();
    } else {
      break;
    }
  }
  if (text.empty()) {
    return std::nullopt;
  }

  // Parse the magnitude in the right base.
  auto base = 10;
  std::string_view digits = text;
  if (text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
    base = 16;
    digits = std::string_view(text).substr(2);
  } else if (text.size() > 2 && text[0] == '0'
             && (text[1] == 'b' || text[1] == 'B')) {
    base = 2;
    digits = std::string_view(text).substr(2);
  } else if (text.size() > 1 && text[0] == '0') {
    base = 8;
    digits = std::string_view(text).substr(1);
  }

  static constexpr auto kUInt64Max = ~static_cast<U128>(0) >> 64;
  const auto overflow = [&]() -> std::optional<SolverValue> {
    if (accept != nullptr)
      accept->error(node, "Conversion overflow for type UInt64.");
    return std::nullopt;
  };
  U128 magnitude = 0;
  for (const auto c : digits) {
    auto d = 0;
    if (c >= '0' && c <= '9') {
      d = c - '0';
    } else if (c >= 'a' && c <= 'f') {
      d = c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
      d = c - 'A' + 10;
    } else {
      return std::nullopt;
    }
    if (d >= base) {
      return std::nullopt;
    }
    if (magnitude
        > (kUInt64Max - static_cast<U128>(d)) / static_cast<U128>(base))
      return overflow();
    magnitude = magnitude * static_cast<U128>(base) + static_cast<U128>(d);
  }

  static constexpr auto kInt64Max = static_cast<U128>(0x7FFFFFFFFFFFFFFFULL);
  static constexpr auto kUInt32Max = static_cast<U128>(0xFFFFFFFFULL);
  static constexpr auto kInt32Max = static_cast<U128>(0x7FFFFFFFULL);

  auto type = PTK::Int32;
  if (magnitude > kInt64Max) {
    type = PTK::UInt64;
  } else if (isUnsigned) {
    type = isLong || magnitude > kUInt32Max ? PTK::UInt64 : PTK::UInt32;
  } else if (isLong || magnitude > kUInt32Max
             || (base == 10 && magnitude > kInt32Max)) {
    type = PTK::Int64;
  } else if (magnitude > kInt32Max) {
    type = PTK::UInt32;
  }
  return makeIntegral(static_cast<I128>(magnitude), type);
}

std::optional<SolverValue> parseFloatLiteral(std::string_view raw) {
  std::string text;
  text.reserve(raw.size());
  for (const auto c : raw) {
    if (c != '\'') {
      text.push_back(c);
    }
  }

  auto type = PTK::Float64;
  if (!text.empty() && (text.back() == 'f' || text.back() == 'F')) {
    type = PTK::Float32;
    text.pop_back();
  }
  // An out-of-range literal reads as zero or infinity; `stod` would throw.
  char* end = nullptr;
  const auto number = std::strtod(text.c_str(), &end);
  if (end == text.c_str()) return std::nullopt;
  return makeFloat(number, type);
}

// --- promotion & operations --------------------------------------------------
// As in C++, a small kind reads as Int32 in any operation.
PTK promoted(PTK kind) {
  switch (kind) {
    case PTK::Int8:
    case PTK::Int16:
    case PTK::UInt8:
    case PTK::UInt16:
      return PTK::Int32;
    default:
      return kind;
  }
}

PTK promote(PTK self, PTK other) {
  self = promoted(self);
  other = promoted(other);
  switch (self) {
    case PTK::Int32:
      return other;
    case PTK::UInt32:
      return other == PTK::Int32 ? PTK::UInt32 : other;
    case PTK::Int64:
    case PTK::Duration:
    case PTK::DateTime:
      return other == PTK::UInt64 ? other : self;
    case PTK::UInt64:
      return PTK::UInt64;
    default:
      return self;
  }
}

std::optional<SolverValue> integralOps(const SolverValue& l,
                                       const SolverValue& r,
                                       std::string_view op) {
  const auto a = trueValue(l);
  const auto b = trueValue(r);
  const auto t = promote(l.type, r.type);
  if (op == "|") return makeIntegral(a | b, t);
  if (op == "&") return makeIntegral(a & b, t);
  if (op == "^") return makeIntegral(a ^ b, t);
  if (op == "+") return makeIntegral(a + b, t);
  if (op == "-") return makeIntegral(a - b, t);
  if (op == "*") {
    // The product wraps at the width of the result. Unsigned arithmetic keeps
    // those low bits without overflowing the signed I128.
    const auto product = static_cast<U128>(a) * static_cast<U128>(b);
    return makeIntegral(static_cast<std::uint64_t>(product), t);
  }
  if (op == "/")
    return b == 0 ? std::nullopt : std::optional{makeIntegral(a / b, t)};
  if (op == "%")
    return b == 0 ? std::nullopt : std::optional{makeIntegral(a % b, t)};
  // A shift has the type of its promoted left operand, whatever the right one.
  const auto shiftType = promoted(l.type);
  if (op == "<<") {
    if (b < 0) return std::nullopt;
    if (b >= 64) return makeIntegral(0, shiftType);
    const auto shifted = static_cast<U128>(a) << static_cast<unsigned>(b);
    return makeIntegral(static_cast<std::uint64_t>(shifted), shiftType);
  }
  if (op == ">>") {
    if (b < 0) return std::nullopt;
    return makeIntegral(
        b >= 64 ? (a < 0 ? -1 : 0) : a >> static_cast<unsigned>(b), shiftType);
  }
  // Comparisons read the true values: unlike C++, a negative value stays below
  // any UInt64.
  if (op == "==") return makeBool(a == b);
  if (op == "!=") return makeBool(a != b);
  if (op == "<") return makeBool(a < b);
  if (op == ">") return makeBool(a > b);
  if (op == "<=") return makeBool(a <= b);
  if (op == ">=") return makeBool(a >= b);
  return std::nullopt;
}

std::optional<SolverValue> floatOps(const SolverValue& l, const SolverValue& r,
                                    std::string_view op) {
  const auto a = l.floating;
  const auto b = r.floating;
  const auto t = (l.type == PTK::Float64 || r.type == PTK::Float64)
                     ? PTK::Float64
                     : PTK::Float32;
  if (op == "+") return makeFloat(a + b, t);
  if (op == "-") return makeFloat(a - b, t);
  if (op == "*") return makeFloat(a * b, t);
  if (op == "/") return makeFloat(a / b, t);
  if (op == "==") return makeBool(a == b);
  if (op == "!=") return makeBool(a != b);
  if (op == "<") return makeBool(a < b);
  if (op == ">") return makeBool(a > b);
  if (op == "<=") return makeBool(a <= b);
  if (op == ">=") return makeBool(a >= b);
  return std::nullopt;
}

std::optional<SolverValue> boolOps(const SolverValue& l, const SolverValue& r,
                                   std::string_view op) {
  if (op == "==") return makeBool(l.boolean == r.boolean);
  if (op == "!=") return makeBool(l.boolean != r.boolean);
  if (op == "<")
    return makeBool(static_cast<int>(l.boolean) < static_cast<int>(r.boolean));
  if (op == ">")
    return makeBool(static_cast<int>(l.boolean) > static_cast<int>(r.boolean));
  if (op == "<=")
    return makeBool(static_cast<int>(l.boolean) <= static_cast<int>(r.boolean));
  if (op == ">=")
    return makeBool(static_cast<int>(l.boolean) >= static_cast<int>(r.boolean));
  return std::nullopt;
}

// `name(arg)`, or nothing for a name outside this fixed set. The caller has
// already stripped the 'f' suffix.
std::optional<double> applyMathFunction(std::string_view name, double arg) {
  if (name == "cos") return std::cos(arg);
  if (name == "sin") return std::sin(arg);
  if (name == "tan") return std::tan(arg);
  if (name == "acos") return std::acos(arg);
  if (name == "asin") return std::asin(arg);
  if (name == "atan") return std::atan(arg);
  if (name == "cosh") return std::cosh(arg);
  if (name == "sinh") return std::sinh(arg);
  if (name == "tanh") return std::tanh(arg);
  if (name == "exp") return std::exp(arg);
  if (name == "log") return std::log(arg);
  if (name == "log10") return std::log10(arg);
  if (name == "expm1") return std::expm1(arg);
  if (name == "log1p") return std::log1p(arg);
  if (name == "sqrt") return std::sqrt(arg);
  if (name == "ceil") return std::ceil(arg);
  if (name == "floor") return std::floor(arg);
  if (name == "abs") return std::fabs(arg);
  return std::nullopt;
}

std::optional<SolverValue> evalValue(const Expression* expression, Ctx& ctx,
                                     Acceptor* accept);

std::optional<SolverValue> evalValueAsType(const Expression* expression,
                                           const Type& type, Ctx& ctx,
                                           Acceptor* accept);

std::optional<SolverValue> applyBinary(const BinaryOperation& expr,
                                       const SolverValue& left,
                                       const SolverValue& right,
                                       Acceptor* accept);

std::optional<SolverValue> applyUnary(const UnaryOperation& expr,
                                      const SolverValue& operand,
                                      Acceptor* accept);

std::optional<SolverValue> checkUnevaluated(const Expression* expression,
                                            Ctx& ctx, Acceptor* accept);

// `&&` and `||` do not evaluate an operand that cannot change their value: it
// is only checked, so that a division by zero there is not an error.
std::optional<SolverValue> evalBinary(const BinaryOperation& expr, Ctx& ctx,
                                      Acceptor* accept) {
  const auto left = evalValue(expr.leftOperand, ctx, accept);
  std::optional<SolverValue> condition;
  if (left && (expr.feature == "&&" || expr.feature == "||"))
    condition = toBool(*left);
  const auto unused = condition && condition->boolean == (expr.feature == "||");
  const auto right = unused ? checkUnevaluated(expr.rightOperand, ctx, accept)
                            : evalValue(expr.rightOperand, ctx, accept);
  if (!left || !right || expr.feature.empty()) {
    return std::nullopt;
  }
  return applyBinary(expr, *left, *right, accept);
}

std::optional<SolverValue> applyBinary(const BinaryOperation& expr,
                                       const SolverValue& left,
                                       const SolverValue& right,
                                       Acceptor* accept) {
  const auto op = expr.feature;

  // Logical operators coerce both operands to bool.
  if (op == "||" || op == "&&") {
    const auto l = toBool(left);
    const auto r = toBool(right);
    if (l && r)
      return makeBool(op == "||" ? (l->boolean || r->boolean)
                                 : (l->boolean && r->boolean));
    // Fall through to the shared unsupported-operator diagnostic.
  }

  // Strings, characters and enumeration literals take no binary operator.
  const auto category = [](const SolverValue& v) -> int {
    if (v.type == PTK::Bool) return 0;
    if (isIntegralType(v.type)) return 1;
    if (isFloatingType(v.type)) return 2;
    return -1;
  };

  const auto lc = category(left);
  const auto rc = category(right);
  std::optional<SolverValue> result;
  if (lc >= 0 && rc >= 0) {
    // Promote to float if either side is float, else to integral if either is
    // integral, else bool.
    if (lc == 2 || rc == 2) {
      const auto ft = lc == 2 ? left.type : right.type;
      const auto l = lc == 2 ? left : *toFloat(left, ft);
      const auto r = rc == 2 ? right : *toFloat(right, ft);
      result = floatOps(l, r, op);
    } else if (lc == 1 || rc == 1) {
      const auto it = lc == 1 ? left.type : right.type;
      const auto l = lc == 1 ? left : *toIntegral(left, it);
      const auto r = rc == 1 ? right : *toIntegral(right, it);
      if ((op == "/" || op == "%") && trueValue(r) == 0) {
        if (accept) accept->error(expr, "Division by zero.");
        return std::nullopt;
      }
      if ((op == "<<" || op == ">>") && trueValue(r) < 0) {
        if (accept) accept->error(expr, "Shift amount must be non-negative.");
        return std::nullopt;
      }
      result = integralOps(l, r, op);
    } else {
      result = boolOps(left, right, op);
    }
  }

  if (!result && accept != nullptr) {
    accept->error(expr, std::string("Binary operator '") + std::string(op)
                            + "' is not supported for operands of type '"
                            + std::string(ptkName(left.type)) + "' and '"
                            + std::string(ptkName(right.type)) + "'.");
  }
  return result;
}

std::optional<SolverValue> evalUnary(const UnaryOperation& expr, Ctx& ctx,
                                     Acceptor* accept) {
  const auto operand = evalValue(expr.operand, ctx, accept);
  if (!operand || expr.feature.empty()) {
    return std::nullopt;
  }
  return applyUnary(expr, *operand, accept);
}

std::optional<SolverValue> applyUnary(const UnaryOperation& expr,
                                      const SolverValue& operand,
                                      Acceptor* accept) {
  const auto op = expr.feature;
  std::optional<SolverValue> result;
  if (op == "!") {
    if (operand.type == PTK::Bool) {
      result = makeBool(!operand.boolean);
    }
  } else if (op == "+") {
    if (isIntegralType(operand.type) || isFloatingType(operand.type)) {
      result = operand;
    }
  } else if (op == "-") {
    if (const auto value = trueValue(operand);
        operand.type == PTK::UInt64
        && value > std::numeric_limits<std::int64_t>::max()) {
      // Such a magnitude reads as UInt64, but its negation is signed, and only
      // the lowest Int64 fits.
      if (value - 1 == std::numeric_limits<std::int64_t>::max()) {
        result = makeIntegral(-value, PTK::Int64);
      } else {
        if (accept != nullptr) {
          accept->error(expr, "Conversion overflow for type Int64.");
        }
        return std::nullopt;
      }
    } else if (isIntegralType(operand.type)) {
      result = makeIntegral(-value, promoted(operand.type));
    } else if (isFloatingType(operand.type)) {
      result = makeFloat(-operand.floating, operand.type);
    }
  } else if (op == "~") {
    if (isIntegralType(operand.type)) {
      result = makeIntegral(~trueValue(operand), promoted(operand.type));
    }
  }

  if (!result && accept != nullptr) {
    accept->error(expr, std::string("Unary operator '") + std::string(op)
                            + "' is not supported for operand of type '"
                            + std::string(ptkName(operand.type)) + "'.");
  }
  return result;
}

// Converts a `BuiltInFunction` call, reporting an unknown function name (once
// the 'f'-suffix float32 spelling has been accounted for) when `accept` is set.
std::optional<SolverValue> convertBuiltinFunction(const BuiltInFunction& func,
                                                  Ctx& ctx, Acceptor* accept) {
  const auto isFloat32 = !func.name.empty() && func.name.back() == 'f';
  const std::string base(isFloat32 ? func.name.substr(0, func.name.size() - 1)
                                   : func.name);
  // Called on 0 only to learn whether the name exists.
  if (!applyMathFunction(base, 0.0)) {
    if (accept)
      accept
          ->error(func,
                  "Unknown built-in function '" + std::string(func.name) + "'.")
          .property<&BuiltInFunction::name>();
    return std::nullopt;
  }

  const auto argValue = evalValue(func.argument, ctx, accept);
  if (!argValue) {
    return std::nullopt;
  }

  const auto arg = toFloat(*argValue, isFloat32 ? PTK::Float32 : PTK::Float64);
  if (!arg) {
    if (accept)
      accept->error(*func.argument,
                    std::string(ptkName(argValue->type))
                        + " cannot be converted to "
                        + (isFloat32 ? "Float32." : "Float64."));
    return std::nullopt;
  }

  return makeFloat(*applyMathFunction(base, arg->floating),
                   isFloat32 ? PTK::Float32 : PTK::Float64);
}

std::optional<SolverValue> evalValue(const Expression* expression, Ctx& ctx,
                                     Acceptor* accept) {
  if (expression == nullptr) {
    return std::nullopt;
  }
  if (const auto* e = cursiv::ast_cast<const IntegerLiteral>(expression)) {
    return parseIntegerLiteral(e->text, *e, accept);
  }
  if (const auto* e = cursiv::ast_cast<const FloatingLiteral>(expression)) {
    return parseFloatLiteral(e->text);
  }
  if (const auto* e = cursiv::ast_cast<const BooleanLiteral>(expression)) {
    return makeBool(e->isTrue);
  }
  if (const auto* e = cursiv::ast_cast<const StringLiteral>(expression)) {
    if (holdsReportedLiteral(*e)) return std::nullopt;
    std::string joined;
    for (const auto& piece : e->value) {
      joined += piece;
    }
    return makeString(std::move(joined));
  }
  if (const auto* e = cursiv::ast_cast<const CharacterLiteral>(expression)) {
    if (holdsReportedLiteral(*e)) return std::nullopt;
    return makeChar(std::string(e->value));
  }

  if (const auto* e = cursiv::ast_cast<const UnaryOperation>(expression)) {
    return evalUnary(*e, ctx, accept);
  }
  if (const auto* e = cursiv::ast_cast<const BinaryOperation>(expression)) {
    return evalBinary(*e, ctx, accept);
  }
  if (const auto* e =
          cursiv::ast_cast<const ParenthesizedExpression>(expression)) {
    return evalValue(e->expr, ctx, accept);
  }

  if (const auto* e = cursiv::ast_cast<const BuiltInConstant>(expression)) {
    if (e->name == "PI") {
      return makeFloat(std::numbers::pi_v<double>, PTK::Float64);
    }
    if (e->name == "E") {
      return makeFloat(std::numbers::e_v<double>, PTK::Float64);
    }
    if (accept != nullptr) {
      accept
          ->error(*e,
                  "Unknown built-in constant '" + std::string(e->name) + "'.")
          .property<&BuiltInConstant::name>();
    }
    return std::nullopt;
  }

  if (const auto* e = cursiv::ast_cast<const BuiltInFunction>(expression)) {
    if (e->argument == nullptr) {
      if (accept != nullptr) {
        accept->error(*e, "Missing argument.")
            .property<&BuiltInFunction::argument>();
      }
    }
    if (e->name.empty()) {
      return std::nullopt;
    }
    return convertBuiltinFunction(*e, ctx, accept);
  }

  if (const auto* e =
          cursiv::ast_cast<const NamedElementReference>(expression)) {
    if (const auto* literal = e->value.get<EnumerationLiteral>()) {
      return makeEnum(literal);
    }
    const auto* constant = e->value.get<Constant>();
    // A name that does not resolve is reported by the linker.
    if (constant == nullptr) {
      return std::nullopt;
    }
    if (accept != nullptr && !isConstantVisibleFrom(*expression, *constant)) {
      accept->error(*expression, "The Constant is not visible.");
    }
    const auto* constantType = constant->type.get();
    if (constantType == nullptr) {
      return std::nullopt;
    }
    if (ctx.evaluating.contains(constant)) {
      ctx.circular = true;
      return std::nullopt;
    }
    if (const auto known = ctx.values.find(constant);
        known != ctx.values.end()) {
      return known->second;
    }
    ctx.evaluating.insert(constant);
    // Do NOT forward `accept`: the Constant's own value is validated once,
    // at its own declaration site (`checkConstant`); re-diagnosing it here
    // would misattribute (or duplicate) the diagnostic at every reference.
    auto result = evalValueAsType(constant->value, *constantType, ctx, nullptr);
    ctx.evaluating.erase(constant);
    ctx.values.emplace(constant, result);
    if (ctx.circular && accept != nullptr) {
      ctx.circular = false;
      accept->error(*expression, "The value of this Constant is circular.");
    }
    return result;
  }

  // What is left has no value of its own: a collection or `nullptr`.
  if (accept != nullptr) {
    accept->error(*expression, isNullptr(expression)
                                   ? nullptrNeedsAPointer
                                   : "A simple constant value is expected.");
  }
  return std::nullopt;
}

// A value of `type` that no check faults, for an operand that is not
// evaluated.
std::optional<SolverValue> typeValue(PTK type) {
  if (isIntegralType(type)) return makeIntegral(1, type);
  if (isFloatingType(type)) return makeFloat(1.0, type);
  if (type == PTK::Bool) return makeBool(true);
  if (type == PTK::String8) return makeString({});
  if (type == PTK::Char8) return makeChar("a");
  return std::nullopt;
}

// Checks an operand that is not evaluated: its names and the types its
// operators take, with values that no check faults, so that nothing that
// depends on a value is reported. A Constant stands for a value of its type,
// whatever its initializer.
std::optional<SolverValue> checkUnevaluated(const Expression* expression,
                                            Ctx& ctx, Acceptor* accept) {
  if (expression == nullptr) return std::nullopt;
  if (const auto* e = cursiv::ast_cast<const NamedElementReference>(expression);
      e && e->value.get<Constant>()) {
    const auto* constant = e->value.get<Constant>();
    if (accept != nullptr && !isConstantVisibleFrom(*expression, *constant))
      accept->error(*expression, "The Constant is not visible.");
    const auto* type = constant->type.get();
    if (const auto* enumeration = cursiv::ast_cast<const Enumeration>(type)) {
      if (enumeration->literal.empty()) return std::nullopt;
      return makeEnum(enumeration->literal.front());
    }
    return type ? typeValue(getPTK(type)) : std::nullopt;
  }
  if (const auto* e =
          cursiv::ast_cast<const ParenthesizedExpression>(expression))
    return checkUnevaluated(e->expr, ctx, accept);
  if (const auto* e = cursiv::ast_cast<const BuiltInFunction>(expression)) {
    const auto isFloat32 = !e->name.empty() && e->name.back() == 'f';
    const std::string base(isFloat32 ? e->name.substr(0, e->name.size() - 1)
                                     : e->name);
    if (!applyMathFunction(base, 0.0)) {
      if (accept)
        accept
            ->error(*e,
                    "Unknown built-in function '" + std::string(e->name) + "'.")
            .property<&BuiltInFunction::name>();
      return std::nullopt;
    }
    if (e->argument == nullptr) {
      if (accept)
        accept->error(*e, "Missing argument.")
            .property<&BuiltInFunction::argument>();
      return std::nullopt;
    }
    const auto kind = isFloat32 ? PTK::Float32 : PTK::Float64;
    const auto argument = checkUnevaluated(e->argument, ctx, accept);
    if (!argument) return std::nullopt;
    if (!toFloat(*argument, kind)) {
      if (accept)
        accept->error(*e->argument,
                      std::string(ptkName(argument->type))
                          + " cannot be converted to "
                          + (isFloat32 ? "Float32." : "Float64."));
      return std::nullopt;
    }
    return typeValue(kind);
  }
  if (const auto* e = cursiv::ast_cast<const BuiltInConstant>(expression)) {
    if (e->name != "PI" && e->name != "E") {
      if (accept)
        accept
            ->error(*e,
                    "Unknown built-in constant '" + std::string(e->name) + "'.")
            .property<&BuiltInConstant::name>();
      return std::nullopt;
    }
    return typeValue(PTK::Float64);
  }
  std::optional<SolverValue> value;
  if (const auto* e = cursiv::ast_cast<const BinaryOperation>(expression)) {
    const auto left = checkUnevaluated(e->leftOperand, ctx, accept);
    const auto right = checkUnevaluated(e->rightOperand, ctx, accept);
    if (left && right && !e->feature.empty())
      value = applyBinary(*e, *left, *right, accept);
  } else if (const auto* e =
                 cursiv::ast_cast<const UnaryOperation>(expression)) {
    const auto operand = checkUnevaluated(e->operand, ctx, accept);
    if (operand && !e->feature.empty())
      value = applyUnary(*e, *operand, accept);
  } else {
    value = evalValue(expression, ctx, accept);
  }
  // Every intermediate result too: `1 / (1 - 1)` is checked as `1 / 1`.
  if (!value) return std::nullopt;
  return typeValue(value->type).value_or(*value);
}

// `value` as a `kind`, or nothing when it does not convert. The bounds of a
// declared type are for `coerceToType`; all that is reported here is a floating
// value that is not finite and a Char8 of other than one byte, which includes
// a UTF-8 character beyond ASCII.
std::optional<SolverValue> coerceToKind(const SolverValue& value, PTK kind,
                                        const Expression& expression,
                                        Acceptor* accept) {
  if (isIntegralType(kind)) {
    return toIntegral(value, kind);
  }
  if (isFloatingType(kind)) {
    auto result = toFloat(value, kind);
    if (result && accept != nullptr && !std::isfinite(result->floating)) {
      accept->error(expression, "Conversion overflow for type "
                                    + std::string(ptkName(kind)) + ".");
    }
    return result;
  }
  if (kind == PTK::Bool) {
    return toBool(value);
  }
  if (kind == PTK::String8) {
    if (value.type == PTK::String8) {
      return value;
    }
    return std::nullopt;
  }
  if (kind == PTK::Char8) {
    auto result = toCharValue(value);
    if (result && accept != nullptr && result->text.size() != 1) {
      accept->error(expression,
                    "A 'Char8' shall contain exactly one character.");
    }
    return result;
  }
  return std::nullopt;
}

// `value` as a value of the declared `type`, or nothing when it does not
// convert. A literal of another Enumeration, a bare integer that names a
// literal, a value outside the declared bounds and a String over its length are
// reported when `accept` is set, and the value is still returned.
std::optional<SolverValue> coerceToType(const SolverValue& value,
                                        const Type& type,
                                        const Expression& expression, Ctx& ctx,
                                        Acceptor* accept) {
  if (const auto* enumeration = cursiv::ast_cast<const Enumeration>(&type)) {
    if (value.type == PTK::Enum) {
      if (accept != nullptr && value.literal != nullptr
          && value.literal->parent != enumeration) {
        accept->error(expression, "Enumeration literal "
                                      + std::string(value.literal->name)
                                      + " is not of type "
                                      + std::string(enumeration->name) + ".");
      }
      return value;
    }
    if (isIntegralType(value.type)) {
      const auto target = trueValue(value);
      for (const auto* literal : enumeration->literal) {
        // Each literal's own value is evaluated as a plain Int32 — NOT `type`
        // (this same Enumeration). Coercing "as `type`" here would re-enter
        // this exact branch for every literal, recursing forever. Silent (no
        // accept): a literal's own value is validated once, at the
        // Enumeration's own declaration site (`checkEnumeration`).
        const auto rawLiteralValue = evalValue(literal->value, ctx, nullptr);
        const auto literalValue =
            rawLiteralValue ? coerceToKind(*rawLiteralValue, PTK::Int32,
                                           *literal->value, nullptr)
                            : std::nullopt;
        if (literalValue && isIntegralType(literalValue->type)
            && trueValue(*literalValue) == target) {
          if (accept != nullptr) {
            accept->warning(expression, "An enumeration literal is expected.")
                .data(diagnosticData(EnumLiteralExpected))
                .relatedInformation(*literal, fqn(*literal));
          }
          return makeEnum(literal);
        }
      }
    }
    return std::nullopt;
  }

  auto result = coerceToKind(value, getPTK(&type), expression, accept);
  if (result && accept) checkTypeBounds(*result, type, expression, *accept);
  return result;
}

std::optional<SolverValue> evalValueAsType(const Expression* expression,
                                           const Type& type, Ctx& ctx,
                                           Acceptor* accept) {
  const auto value = evalValue(expression, ctx, accept);
  if (!value) {
    return std::nullopt;
  }
  return coerceToType(*value, type, *expression, ctx, accept);
}

}  // namespace

namespace {

// `std::to_string` has no overload for 128 bits.
std::string decimalText(I128 value) {
  if (value == 0) return "0";
  std::string out;
  for (auto rest = value < 0 ? -value : value; rest > 0; rest /= 10)
    out.insert(out.begin(),
               static_cast<char>('0' + static_cast<int>(rest % 10)));
  return value < 0 ? "-" + out : out;
}

// How a diagnostic writes an integral or floating value: an integer in full, a
// floating value with the fewest digits that read back the same.
std::string valueText(const SolverValue& value) {
  if (isFloatingType(value.type)) {
    char buffer[32];
    const auto end =
        std::to_chars(std::begin(buffer), std::end(buffer), value.floating).ptr;
    return std::string(buffer, end);
  }
  return isSignedIntegral(value.type)
             ? std::to_string(static_cast<std::int64_t>(value.integral))
             : std::to_string(value.integral);
}

// A bound may stand in another document than what is checked against it.
std::string_view uriOf(const cursiv::AstNode& node) {
  const auto* document = node.getDocument();
  return document ? std::string_view(document->uri) : std::string_view{};
}

std::string conversionError(const SolverValue& value, PTK kind,
                            const std::string& target) {
  if (value.type == PTK::String8 && kind == PTK::DateTime)
    return "Invalid DateTime, expected a value such as 1970-01-01T00:00:00Z.";
  if (value.type == PTK::String8 && kind == PTK::Duration)
    return "Invalid Duration, expected a value such as PT1S.";
  if (isIntegralType(value.type) && isIntegralType(kind)) {
    const auto width = integralWidth(kind);
    const auto isSigned = isSignedIntegral(kind);
    const auto top = I128{1} << (isSigned ? width - 1 : width);
    return decimalText(trueValue(value)) + " is out of range for " + target
           + " (" + decimalText(isSigned ? -top : I128{0}) + " to "
           + decimalText(top - 1) + ").";
  }
  return std::string(ptkName(value.type)) + " cannot be converted to " + target
         + ".";
}

}  // namespace

bool isValidDuration(std::string_view text) {
  return durationValue(text).has_value();
}

std::optional<std::int64_t> durationNanoseconds(std::string_view text) {
  const auto value = durationValue(text);
  return value ? integralValueOf(*value) : std::nullopt;
}

bool isValidDateTime(std::string_view text) {
  return dateTimeValue(text).has_value();
}

std::optional<std::int64_t> parseDateTimeMilliseconds(std::string_view text) {
  const auto nanoseconds = parseDateTimeString(text);
  if (!nanoseconds) return std::nullopt;
  auto milliseconds = *nanoseconds / 1'000'000;
  if (*nanoseconds < 0 && *nanoseconds % 1'000'000) --milliseconds;
  return static_cast<std::int64_t>(milliseconds);
}

void checkTypeBounds(const SolverValue& value, const Type& type,
                     const cursiv::AstNode& node, Acceptor& accept) {
  // A bound's own errors are reported where its type is declared: silent here.
  Ctx ctx;
  const auto kind = getPTK(&type);
  const auto convertedBound =
      [&](const Expression* bound) -> std::optional<SolverValue> {
    const auto raw = evalValue(bound, ctx, nullptr);
    return raw ? coerceToKind(*raw, kind, *bound, nullptr) : std::nullopt;
  };

  if (const auto* integer = cursiv::ast_cast<const Integer>(&type)) {
    if (const auto min = convertedBound(integer->minimum);
        min && trueValue(value) < trueValue(*min)) {
      const auto text = valueText(*min);
      accept
          .error(node, "Integral value shall be greater than or equal to "
                           + text + ".")
          .relatedInformation(*integer->minimum, text,
                              uriOf(*integer->minimum));
    }
    if (const auto max = convertedBound(integer->maximum);
        max && trueValue(value) > trueValue(*max)) {
      const auto text = valueText(*max);
      accept
          .error(node,
                 "Integral value shall be less than or equal to " + text + ".")
          .relatedInformation(*integer->maximum, text,
                              uriOf(*integer->maximum));
    }
  } else if (const auto* floating = cursiv::ast_cast<const Float>(&type)) {
    const auto minExclusive =
        floating->range == "<.." || floating->range == "<.<";
    const auto maxExclusive =
        floating->range == "..<" || floating->range == "<.<";
    if (const auto min = convertedBound(floating->minimum);
        min
        && (value.floating < min->floating
            || (minExclusive && value.floating == min->floating))) {
      const auto text = valueText(*min);
      accept
          .error(node, "Float value shall be greater than "
                           + std::string(minExclusive ? "" : "or equal to ")
                           + text + ".")
          .relatedInformation(*floating->minimum, text,
                              uriOf(*floating->minimum));
    }
    if (const auto max = convertedBound(floating->maximum);
        max
        && (value.floating > max->floating
            || (maxExclusive && value.floating == max->floating))) {
      const auto text = valueText(*max);
      accept
          .error(node, "Float value shall be less than "
                           + std::string(maxExclusive ? "" : "or equal to ")
                           + text + ".")
          .relatedInformation(*floating->maximum, text,
                              uriOf(*floating->maximum));
    }
  } else if (const auto* string = cursiv::ast_cast<const String>(&type)) {
    const auto rawLength = evalValue(string->length, ctx, nullptr);
    const auto length =
        rawLength ? toIntegral(*rawLength, PTK::Int64) : std::nullopt;
    if (length && static_cast<I128>(value.text.size()) > trueValue(*length))
      accept
          .error(node,
                 "The string length exceeds the allowed length for its type.")
          .relatedInformation(type, valueText(*length), uriOf(type));
  }
}

bool isNullptr(const Expression* expression) {
  if (const auto* parenthesized =
          cursiv::ast_cast<const ParenthesizedExpression>(expression))
    return isNullptr(parenthesized->expr);
  return cursiv::ast_cast<const KeywordExpression>(expression) != nullptr;
}

PTK getPTK(const Type* type, PTK defaultKind) {
  if (type) {
    if (const auto* primitive = cursiv::ast_cast<const PrimitiveType>(type)) {
      static constexpr std::pair<std::string_view, PTK> kinds[] = {
          {"Smp.Bool", PTK::Bool},         {"Smp.Char8", PTK::Char8},
          {"Smp.DateTime", PTK::DateTime}, {"Smp.Duration", PTK::Duration},
          {"Smp.Float32", PTK::Float32},   {"Smp.Float64", PTK::Float64},
          {"Smp.Int8", PTK::Int8},         {"Smp.Int16", PTK::Int16},
          {"Smp.Int32", PTK::Int32},       {"Smp.Int64", PTK::Int64},
          {"Smp.UInt8", PTK::UInt8},       {"Smp.UInt16", PTK::UInt16},
          {"Smp.UInt32", PTK::UInt32},     {"Smp.UInt64", PTK::UInt64},
          {"Smp.String8", PTK::String8}};
      for (const auto& [name, kind] : kinds)
        if (name.ends_with(primitive->name))
          return hasQualifiedName(*primitive, name) ? kind : PTK::None;
      return PTK::None;
    }

    // The grammar only links a Float or an Integer to a primitive type.
    if (const auto* floatType = cursiv::ast_cast<const Float>(type)) {
      return getPTK(
          floatType->primitiveType ? floatType->primitiveType->get() : nullptr,
          PTK::Float64);
    }
    if (const auto* integerType = cursiv::ast_cast<const Integer>(type)) {
      return getPTK(integerType->primitiveType
                        ? integerType->primitiveType->get()
                        : nullptr,
                    PTK::Int32);
    }
    if (cursiv::ast_cast<const String>(type) != nullptr) {
      return PTK::String8;
    }
    if (cursiv::ast_cast<const Enumeration>(type) != nullptr) {
      return PTK::Enum;
    }
    return PTK::None;
  }
  return defaultKind;
}

std::optional<SolverValue> getValue(const Expression* expression,
                                    Acceptor* accept) {
  Ctx ctx;
  return evalValue(expression, ctx, accept);
}

std::optional<SolverValue> getValueAs(const Expression* expression,
                                      const Type& type, Acceptor* accept) {
  if (expression == nullptr) {
    return std::nullopt;
  }
  Ctx ctx;
  const auto value = evalValue(expression, ctx, accept);
  if (!value) {
    return std::nullopt;
  }
  auto result = coerceToType(*value, type, *expression, ctx, accept);
  if (!result && accept != nullptr) {
    accept->error(*expression,
                  conversionError(*value, getPTK(&type), fqn(type)));
  }
  return result;
}

std::optional<SolverValue> getValueAs(const Expression* expression, PTK kind,
                                      Acceptor* accept) {
  if (expression == nullptr) {
    return std::nullopt;
  }
  Ctx ctx;
  const auto value = evalValue(expression, ctx, accept);
  if (!value) {
    return std::nullopt;
  }
  auto result = coerceToKind(*value, kind, *expression, accept);
  if (!result && accept != nullptr) {
    accept->error(*expression,
                  conversionError(*value, kind, std::string(ptkName(kind))));
  }
  return result;
}

std::optional<std::int64_t> integralValueOf(const SolverValue& value) {
  if (!isIntegralType(value.type)) {
    return std::nullopt;
  }
  return static_cast<std::int64_t>(value.integral);
}

std::optional<double> floatingValueOf(const SolverValue& value) {
  if (!isFloatingType(value.type)) {
    return std::nullopt;
  }
  return value.floating;
}

}  // namespace xsmp
