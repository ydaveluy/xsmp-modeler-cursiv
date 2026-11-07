#include <string>
#include <xsmp/esa-cdk/Scaffold.hpp>
#include <xsmp/utils/FileWriter.hpp>

namespace xsmp {

void scaffoldEsaCdkProject(const std::filesystem::path& project,
                           std::string_view name) {
  const auto path = project / "CMakeLists.txt";
  writeFileIfMissing(
      path,
      "file(GLOB_RECURSE SRC CONFIGURE_DEPENDS src/*.cpp "
      "src-gen/*.cpp)\n\nsimulus_library(\n    MAIN\n    SOURCES\n        "
      "${SRC}\n    LIBRARIES\n        "
      "esa.ecss.smp.cdk\n)\ntarget_include_directories("
          + std::string(name) + " PUBLIC src src-gen)\n");
}

}  // namespace xsmp
