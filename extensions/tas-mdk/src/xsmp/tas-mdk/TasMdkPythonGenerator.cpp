#include <algorithm>
#include <cctype>
#include <cursiv/text/TextBuilder.hpp>
#include <xsmp/CopyrightNoticeProvider.hpp>
#include <xsmp/ProjectCreation.hpp>
#include <xsmp/references/QualifiedName.hpp>
#include <xsmp/tas-mdk/TasMdkPythonGenerator.hpp>
#include <xsmp/utils/FileWriter.hpp>

namespace xsmp {

namespace {

// Splits `a.b.C` into `a.b` and `C`. The first part is `TBD` without a dot.
std::pair<std::string, std::string> splitName(const std::string& name) {
  const auto dot = name.rfind('.');
  if (dot == std::string::npos) return {"TBD", name};
  return {name.substr(0, dot), name.substr(dot + 1)};
}

// The content of a file that only holds the notice.
std::string noticeFile(const std::optional<std::string>& notice) {
  cursiv::TextBuilder out;
  if (notice) out.write(*notice);
  return out.take();
}

// `label = [...]`: the names of the elements of kind `T` of `type`, as a list
// of JSON strings.
template <class T>
void writeNames(cursiv::TextBuilder& out, std::string_view label,
                const Component& type) {
  out.write(label, " = [");
  std::string_view separator;
  for (const auto* element : type.elements)
    if (cursiv::ast_cast<const T>(element)) {
      out.write(separator, "\"", element->name, "\"");
      separator = ",";
    }
  out.line("]");
}

}  // namespace

void TasMdkPythonGenerator::generateFile(const std::filesystem::path& project,
                                         const std::filesystem::path& relative,
                                         std::string_view content,
                                         bool preserve) const {
  const auto path = project / "helpers" / relative;
  if (preserve)
    writeFileIfMissing(path, content);
  else
    writeFileIfChanged(path, content);
}

void TasMdkPythonGenerator::generate(
    const Catalogue& catalogue, const std::filesystem::path& project) const {
  const auto notice =
      getCopyrightNotice(*services.commentProvider, catalogue, "# ");
  const auto init = noticeFile(notice);
  for (const auto* folder : {"", "builder", "test_utils"})
    generateFile(project, std::filesystem::path(folder) / "__init__.py", init,
                 true);
  for (const auto* ns : catalogue.elements)
    generateNamespace(*ns, project, notice, catalogue.name);
}

void TasMdkPythonGenerator::generateNamespace(
    const Namespace& ns, const std::filesystem::path& project,
    const std::optional<std::string>& notice,
    std::string_view libraryName) const {
  auto path = qualifiedName(ns);
  std::ranges::replace(path, '.', '/');
  const auto init = noticeFile(notice);
  for (const auto* folder : {"builder", "test_utils"})
    generateFile(project, std::filesystem::path(folder) / path / "__init__.py",
                 init, true);
  for (const auto* element : ns.elements) {
    if (const auto* nested = cursiv::ast_cast<const Namespace>(element))
      generateNamespace(*nested, project, notice, libraryName);
    else if (const auto* component = cursiv::ast_cast<const Component>(element))
      generateComponent(*component, project, notice, libraryName);
  }
}

void TasMdkPythonGenerator::generateComponent(
    const Component& type, const std::filesystem::path& project,
    const std::optional<std::string>& notice,
    std::string_view libraryName) const {
  const auto qualified = qualifiedName(type);
  auto path = qualified;
  std::ranges::replace(path, '.', '/');
  const auto builder = std::filesystem::path("builder") / path;
  const auto tests = std::filesystem::path("test_utils") / path;

  std::string library(libraryName);
  std::ranges::transform(library, library.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  {
    cursiv::TextBuilder out;
    if (notice) out.write(*notice).line();
    out.line();
    out.line("# Import user-defined methods for model integration");
    out.line("from .user_code import configureInstance, extendInstance");
    out.line();
    out.line("# Model specific data");
    out.line("MODEL_UUID = \"", docTag(type, "uuid").value_or(""), "\"");
    out.line("MODEL_LIB_NAME = \"lib", library, ".so\"");
    out.line();
    generateFile(project, builder / "generated_info.py", out.text());
  }
  {
    cursiv::TextBuilder out;
    if (notice) out.write(*notice).line();
    out.line();
    out.line("# Model specific data");
    out.write("IMPLEMENTATIONS = [");
    std::string_view separator;
    for (const auto& interface : type.interface_)
      if (const auto name = qualifiedName(*interface.get());
          !name.starts_with("Smp.")) {
        out.write(separator, "\"", name, "\"");
        separator = ",";
      }
    out.line("]");
    out.write("REFERENCES = [");
    separator = {};
    for (const auto* element : type.elements)
      if (const auto* reference = cursiv::ast_cast<const Reference>(element)) {
        out.write(separator, "{\"name\":\"", reference->name, "\",\"kind\":\"",
                  qualifiedName(*reference->interface_.get()), "\"}");
        separator = ",";
      }
    out.line("]");
    writeNames<EventSink>(out, "EVENTSINKS", type);
    writeNames<EventSource>(out, "EVENTSOURCES", type);
    generateFile(project, builder / "generated_icd.py", out.text());
  }
  {
    cursiv::TextBuilder out;
    if (notice) out.write(*notice).line();
    out.write(R"(
def configureInstance(jsim, instance_data):
    pass

def extendInstance(jsim, instance_data):
    pass
)");
    generateFile(project, builder / "user_code.py", out.text(), true);
  }
  generateFile(project, builder / "__init__.py", noticeFile(notice), true);

  // The wrapper derives from the wrapper of the base class, found in the
  // Python package of that base. The `TasMdk` bases live in two packages.
  std::string baseName = "Model", basePath = "TasMdk.Model",
              baseRoot = "TasMdk__CommonModels";
  if (type.base.has_value()) {
    const auto [module, moduleName] =
        splitName(qualifiedName(*type.base->get()));
    baseName = moduleName;
    basePath = module + "." + moduleName;
    baseRoot = module.substr(0, module.find('.'));
    if (baseRoot == "TasMdk") {
      if (basePath.starts_with("TasMdk.CommonModels")
          || basePath.starts_with("TasMdk.InterfacesImplem")
          || basePath.starts_with("TasMdk.Model"))
        baseRoot = "TasMdk__CommonModels";
      else if (basePath.starts_with("TasMdk.CommonTestModels"))
        baseRoot = "TasMdk__CommonTestModels";
    }
  }
  const auto own = splitName(qualified);
  {
    cursiv::TextBuilder out;
    if (notice) out.write(*notice).line();
    out.line("from ", baseRoot, ".test_utils.", basePath, " import ", baseName,
             "Wrapper");
    out.line("from ", libraryName, ".builder.", own.first, " import ",
             own.second);
    out.line();
    out.line("class ", type.name, "Wrapper( ", baseName, "Wrapper ):");
    out.line("    builder = ", type.name);
    generateFile(project, tests / "wrapper.py", out.text(), true);
  }
  {
    cursiv::TextBuilder out;
    if (notice) out.write(*notice).line();
    out.line("from .wrapper import ", type.name, "Wrapper");
    generateFile(project, tests / "__init__.py", out.text(), true);
  }
}

void scaffoldTasMdkProject(const std::filesystem::path& project,
                           std::string_view projectName) {
  const auto write = [&](std::string_view relative, std::string content) {
    writeFileIfMissing(project / relative, content);
  };
  // As named by `TasMdkPythonGenerator`, from the catalogue.
  const auto identifier = projectIdentifier(projectName);
  auto library = identifier;
  std::ranges::transform(library, library.begin(),
                         [](unsigned char c) { return std::tolower(c); });
  write("Makefile", "COMPONENT_NAME=" + std::string(projectName) + R"(

ifndef COMPILE_CHAIN
$(error Variable COMPILE_CHAIN is not defined)
endif

-include $(COMPILE_CHAIN)/component_declaration.mk

CXX_STANDARD=11

LIBRARY_NAME=lib)" + library
                        +
                        R"(.so

include $(COMPILE_CHAIN)/rules_libs.mk

$(COMPONENT_NAME)_tests:
	$(MAKE) -C tests
	$(MAKE) -C tests tu
)");
  write("component.conf",
        "__LIBRARIES__=TasMdk\n__REFERENCED_LIBRARIES__=\n__INCLUDE_FOLDERS__="
        "include,include-gen\n__SOURCE_FOLDERS__=src,src-gen\n__OPERATIONAL_"
        "PYTHON_TOOLS_FOLDER__=helpers\n__VERSION__=1\n");
  write("tests/Makefile",
        "COMPONENT_NAME=" + std::string(projectName) + R"(--tests

ifndef COMPILE_CHAIN
$(error Variable COMPILE_CHAIN is not defined)
endif

-include $(COMPILE_CHAIN)/component_declaration.mk
include $(COMPILE_CHAIN)/rules_variants.mk

$(COMPONENT_NAME)_tests: $(COMPONENT_NAME)_tests_python
$(COMPONENT_NAME)_tests_python:
	COMPONENT_NAME=$(COMPONENT_NAME) \
	VT_STDOUT=$(VT_STDOUT) \
	PATH=${ROOT_OBJ}/gram_addons--simulator_launcher/BIN:${PATH} \
	PYTHONPATH=${PYTHONPATH} \
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH} \
	python3 -m gram_addons__python_test_suite.runtests $(TEST_ARGS)
)");
  write("tests/component.conf",
        "__LIBRARIES__=\\\ngram_addons--python_test_suite,\\\ngram_addons--"
        "simulator_launcher,\\\n"
            + std::string(projectName) + "\n__COMPONENTS_ROOT_PATH__=.\n");
  write(
      "tests/ut_" + identifier + ".py",
      R"(from gram_addons__python_test_suite.gram_test_case import GramTestCase, SECOND

class MyModelTestCase(GramTestCase):
    def buildSimulatorData(self, jsim):
        pass

    def test_model_feature1(self):
        self.sim.run_delta(SECOND)
)");
}

}  // namespace xsmp
