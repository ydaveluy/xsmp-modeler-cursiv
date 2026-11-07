#pragma once

#include <lsp/json/json.h>

#include <algorithm>
#include <cctype>
#include <cursiv/services/JsonValue.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

// Helpers for the JSON of manifests and of the messages to workers.

namespace xsmp {

using Json = ::lsp::json::Value;
using JsonObject = ::lsp::json::Object;
using JsonArray = ::lsp::json::Array;

inline cursiv::JsonValue toCoreJson(const Json& value) {
  if (value.isBoolean()) return value.boolean();
  if (value.isInteger()) return static_cast<std::int64_t>(value.integer());
  if (value.isDecimal()) return value.decimal();
  if (value.isString()) return value.string();
  if (value.isArray()) {
    cursiv::JsonValue::Array result;
    for (const auto& element : value.array())
      result.push_back(toCoreJson(element));
    return result;
  }
  if (value.isObject()) {
    cursiv::JsonValue::Object result;
    for (const auto& [key, element] : value.object())
      result.emplace(key, toCoreJson(element));
    return result;
  }
  return nullptr;
}

// Both give `fallback` when the key is missing or holds another type.
inline std::string stringAt(const JsonObject& object, std::string_view key,
                            std::string fallback = {}) {
  const auto* value = object.find(key);
  return value && value->isString() ? value->string() : std::move(fallback);
}

inline bool boolAt(const JsonObject& object, std::string_view key,
                   bool fallback = false) {
  const auto* value = object.find(key);
  return value && value->isBoolean() ? value->boolean() : fallback;
}

// Empty when the key is missing or no array. Throws on a non-string item.
inline std::vector<std::string> stringsAt(const JsonObject& object,
                                          std::string_view key) {
  std::vector<std::string> values;
  if (const auto* array = object.find(key); array && array->isArray())
    for (const auto& item : array->array()) {
      if (!item.isString())
        throw std::invalid_argument(std::string(key) + " must contain strings");
      values.push_back(item.string());
    }
  return values;
}

// The bytes of a file. Throws when it cannot be read.
inline std::string readText(const std::filesystem::path& path) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream) throw std::runtime_error("Cannot read " + path.string());
  std::ostringstream contents;
  contents << stream.rdbuf();
  return contents.str();
}

// Whether `path` is `root` or lies under it. Only the text of the paths counts:
// links are not followed, so give normalized paths.
inline bool containsPath(const std::filesystem::path& root,
                         const std::filesystem::path& path) {
  const auto relative = path.lexically_relative(root);
  return !relative.empty() && !relative.is_absolute()
         && *relative.begin() != "..";
}

}  // namespace xsmp
