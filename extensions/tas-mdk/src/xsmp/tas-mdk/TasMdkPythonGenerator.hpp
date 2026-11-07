#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <xsmp/XsmpcatCoreServices.hpp>
#include <xsmp/utils/XsmpDocumentation.hpp>

namespace xsmp {

// Writes the Python helpers of a project under `helpers/`: for each component,
// its packages in `builder/` and `test_utils/`. Data files are always
// rewritten; the files a user may edit are written once.
class TasMdkPythonGenerator final {
 public:
  explicit TasMdkPythonGenerator(const XsmpcatCoreServices& services)
      : services(services) {}

  void generate(const Catalogue&, const std::filesystem::path&) const;

 private:
  // `preserve` keeps a file that exists, which the user may have edited.
  void generateFile(const std::filesystem::path& project,
                    const std::filesystem::path& relative,
                    std::string_view content, bool preserve = false) const;

  void generateNamespace(const Namespace&, const std::filesystem::path& project,
                         const std::optional<std::string>& notice,
                         std::string_view libraryName) const;

  void generateComponent(const Component&, const std::filesystem::path& project,
                         const std::optional<std::string>& notice,
                         std::string_view libraryName) const;

  const XsmpcatCoreServices& services;
};

// Writes the starter files of a new project. A file that exists is kept.
void scaffoldTasMdkProject(const std::filesystem::path&,
                           std::string_view projectName);

}  // namespace xsmp
