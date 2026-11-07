#pragma once

#include <charconv>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

namespace xsmp {

/// The time that generated files must name for a reproducible build: the
/// standard `SOURCE_DATE_EPOCH`, in seconds since the epoch. None when it is
/// not set, and generated files then name the present.
[[nodiscard]] inline std::optional<std::time_t> reproducibleTime() {
  const auto* text = std::getenv("SOURCE_DATE_EPOCH");
  if (!text) return std::nullopt;
  const std::string_view digits(text);
  std::int64_t seconds = 0;
  const auto* const last = digits.data() + digits.size();
  const auto [end, error] = std::from_chars(digits.data(), last, seconds);
  if (error != std::errc{} || end != last) return std::nullopt;
  return static_cast<std::time_t>(seconds);
}

[[nodiscard]] inline std::tm utcCalendar(std::time_t time) {
  std::tm calendar{};
#ifdef _WIN32
  gmtime_s(&calendar, &time);
#else
  gmtime_r(&time, &calendar);
#endif
  return calendar;
}

[[nodiscard]] inline std::tm localCalendar(std::time_t time) {
  std::tm calendar{};
#ifdef _WIN32
  localtime_s(&calendar, &time);
#else
  localtime_r(&time, &calendar);
#endif
  return calendar;
}

/// The name of the user who generates a file.
[[nodiscard]] inline std::string generationUser() {
  const auto* user = std::getenv("USER");
  if (!user) user = std::getenv("USERNAME");
  return user ? user : "unknown";
}

}  // namespace xsmp
