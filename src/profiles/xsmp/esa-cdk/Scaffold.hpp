#pragma once

#include <filesystem>
#include <string_view>

namespace xsmp {

// Writes a starting `CMakeLists.txt` in the project folder, unless there is
// one. It builds `src` and `src-gen` against the ESA CDK.
void scaffoldEsaCdkProject(const std::filesystem::path&,
                           std::string_view projectName);
}  // namespace xsmp
