#pragma once

#include <cstdint>
#include <string_view>

namespace xsmp {

/// Used by the constant expression solver to classify and convert values.
enum class PTK : std::uint8_t {
  Bool,
  Char8,
  Float32,
  Float64,
  Int8,
  Int16,
  Int32,
  Int64,
  UInt8,
  UInt16,
  UInt32,
  UInt64,
  // A number of nanoseconds in 64 bits, from `kDateTimeEpochSeconds` for a
  // DateTime.
  DateTime,
  Duration,
  String8,
  // A type that has no primitive kind, such as a Structure.
  None,
  // An Enumeration: its values are literals rather than numbers.
  Enum,
};

[[nodiscard]] constexpr bool isIntegralType(PTK kind) noexcept {
  switch (kind) {
    case PTK::Int8:
    case PTK::Int16:
    case PTK::Int32:
    case PTK::Int64:
    case PTK::UInt8:
    case PTK::UInt16:
    case PTK::UInt32:
    case PTK::UInt64:
    case PTK::DateTime:
    case PTK::Duration:
      return true;
    default:
      return false;
  }
}

[[nodiscard]] constexpr bool isFloatingType(PTK kind) noexcept {
  return kind == PTK::Float32 || kind == PTK::Float64;
}

/// Whether an integral kind is signed (UInt* are unsigned; DateTime/Duration
/// are signed 64-bit).
[[nodiscard]] constexpr bool isSignedIntegral(PTK kind) noexcept {
  switch (kind) {
    case PTK::UInt8:
    case PTK::UInt16:
    case PTK::UInt32:
    case PTK::UInt64:
      return false;
    default:
      return true;
  }
}

/// Bit width of an integral kind.
[[nodiscard]] constexpr int integralWidth(PTK kind) noexcept {
  switch (kind) {
    case PTK::Int8:
    case PTK::UInt8:
      return 8;
    case PTK::Int16:
    case PTK::UInt16:
      return 16;
    case PTK::Int32:
    case PTK::UInt32:
      return 32;
    default:
      return 64;
  }
}

[[nodiscard]] std::string_view ptkName(PTK kind) noexcept;

}  // namespace xsmp
