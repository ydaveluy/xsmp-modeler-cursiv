#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <xsmp/XsmpcatCoreServices.hpp>
#include <xsmp/ast.hpp>
#include <xsmp/utils/XsmpDocumentation.hpp>

namespace xsmp {

// Writes a Python package for a catalogue: one `__init__.py` per namespace,
// with a class that holds the UUID of each Component.
class PythonGenerator {
 public:
  explicit PythonGenerator(const XsmpcatCoreServices& services)
      : services(services) {}

  // Writes the package under `destination/python`.
  void generate(const Catalogue& catalogue,
                const std::filesystem::path& destination) const;

 private:
  const XsmpcatCoreServices& services;
  void generateNamespace(const Namespace&, const std::filesystem::path&,
                         const std::optional<std::string>& notice) const;
};

}  // namespace xsmp
