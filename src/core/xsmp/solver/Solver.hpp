#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <xsmp/solver/PrimitiveTypeKind.hpp>

namespace xsmp {

struct Expression;
struct Type;
struct EnumerationLiteral;

}  // namespace xsmp

namespace cursiv {

struct AstNode;
class ValidationAcceptor;

}  // namespace cursiv

namespace xsmp {

using Acceptor = cursiv::ValidationAcceptor;

/// An SMP DateTime counts nanoseconds from 2000-01-01T12:00:00Z
/// (ECSS-E-ST-40-07 5.2.1), which is this many seconds after the Unix epoch.
inline constexpr std::int64_t kDateTimeEpochSeconds = 946'728'000;

/// An `xs:duration` with days, hours, minutes and seconds, such as `PT1S` or
/// `-P2DT1M`, that a Duration holds.
[[nodiscard]] bool isValidDuration(std::string_view text);
/// The value of a valid Duration text, in nanoseconds.
[[nodiscard]] std::optional<std::int64_t> durationNanoseconds(
    std::string_view text);
/// An `xsd:dateTime` such as `1970-01-01T00:00:00Z` that a DateTime holds:
/// roughly between 1710 and 2290.
[[nodiscard]] bool isValidDateTime(std::string_view text);
/// The milliseconds since the Unix epoch of any `xsd:dateTime`, rounded towards
/// minus infinity.
[[nodiscard]] std::optional<std::int64_t> parseDateTimeMilliseconds(
    std::string_view text);

/// A compile-time constant value. `type` selects which payload field is live:
/// `boolean` for Bool, `integral` (a wrapped bit pattern) for the integral
/// kinds, `floating` for Float32/Float64, `text` for String8/Char8, `literal`
/// for Enum.
struct SolverValue {
  PTK type = PTK::None;
  std::uint64_t integral = 0;
  double floating = 0.0;
  bool boolean = false;
  std::string text;
  const EnumerationLiteral* literal = nullptr;
};

/// The primitive kind of `type` (resolving Integer/Float to their storage
/// primitive, builtin primitives by fully-qualified name). `defaultKind`
/// answers for a null `type`: an Integer without a primitive reads as Int32 and
/// a Float as Float64.
[[nodiscard]] PTK getPTK(const Type* type, PTK defaultKind = PTK::None);

/// Whether `expression` is `nullptr`, perhaps in parentheses: it has no value
/// for `getValue`, and is only valid for a pointer.
[[nodiscard]] bool isNullptr(const Expression* expression);

/// What is said of `nullptr` where the C++ type is not a pointer.
inline constexpr const char* nullptrNeedsAPointer =
    "The nullptr literal requires a pointer type.";

/// Evaluates a constant expression, or `std::nullopt` when it is not a
/// compile-time constant (or evaluation fails, e.g. division by zero or a
/// circular constant reference). `accept`, when non-null, receives diagnostics
/// discovered DURING evaluation (what has no value, unknown built-in
/// constant/function, a missing built-in-function argument, a circular or
/// non-visible Constant reference). A Constant whose own value fails is
/// reported where it is declared. Omit it (the default) for a purely silent
/// evaluation, e.g. when re-deriving a value for further validator-side
/// comparison where the underlying expression's own diagnostics have already
/// been (or will be) reported at its own declaration site.
[[nodiscard]] std::optional<SolverValue> getValue(const Expression* expression,
                                                  Acceptor* accept = nullptr);

/// Evaluates `expression` and converts it to `type` (Integer/Float/String/Char/
/// Bool/Enumeration). When `accept` is non-null, also reports: out-of-range
/// Integer/Float values, a String8/Char8 length violation, an Enumeration
/// value that doesn't match any literal (error) or that matches one but was
/// given as a bare integer (warning), plus everything `getValue` itself may
/// report while evaluating `expression`.
[[nodiscard]] std::optional<SolverValue> getValueAs(
    const Expression* expression, const Type& type, Acceptor* accept = nullptr);

// The same for a bare kind, which has no declared bounds. In both overloads a
// value that does not convert is reported and gives nothing, while one outside
// its declared bounds is reported and still returned.
[[nodiscard]] std::optional<SolverValue> getValueAs(
    const Expression* expression, PTK kind, Acceptor* accept = nullptr);

/// Reports on `node` a `value`, already of the kind of `type`, outside the
/// bounds of an Integer or a Float, or too long for a String. Any other type
/// has no bounds.
void checkTypeBounds(const SolverValue& value, const Type& type,
                     const cursiv::AstNode& node, Acceptor& accept);

/// Signed integer view of an integral `SolverValue` (for validation
/// comparisons), or `std::nullopt` if it is not integral. A UInt64 above the
/// Int64 range reads as negative.
[[nodiscard]] std::optional<std::int64_t> integralValueOf(
    const SolverValue& value);

/// Floating-point view of a `SolverValue`, or `std::nullopt` if it is not
/// floating.
[[nodiscard]] std::optional<double> floatingValueOf(const SolverValue& value);

}  // namespace xsmp
