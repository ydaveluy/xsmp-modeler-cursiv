#include <xsmp/utils/FileWriter.hpp>

#include "Contributions.hpp"
#include "ProjectCreation.hpp"

namespace xsmp {

namespace {

// Expands `@PROJECT_NAME@` and `@PROJECT_IDENTIFIER@` in a template.
std::string render(std::string text, std::string_view name) {
  const auto identifier = projectIdentifier(name);
  for (const auto& [key, value] :
       {std::pair<std::string_view, std::string_view>{"@PROJECT_NAME@", name},
        {"@PROJECT_IDENTIFIER@", identifier}}) {
    for (std::size_t at = 0; (at = text.find(key, at)) != std::string::npos;
         at += value.size())
      text.replace(at, key.size(), value);
  }
  return text;
}

}  // namespace

void scaffoldXsmpSdkProject(const std::filesystem::path& directory,
                            std::string_view name) {
  {
    const auto file = directory / render("CMakeLists.txt", name);
    writeFileIfMissing(file,
                       render(R"XSMP_TPL(cmake_minimum_required(VERSION 3.14)

project(
    @PROJECT_NAME@
#   VERSION 1.0.0
#   DESCRIPTION ""
#   HOMEPAGE_URL ""
    LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    xsmp-sdk
    GIT_REPOSITORY https://github.com/ThalesGroup/xsmp-sdk.git
    GIT_TAG        main # replace with a specific tag
)
FetchContent_MakeAvailable(xsmp-sdk)
list(APPEND CMAKE_MODULE_PATH "${xsmp-sdk_SOURCE_DIR}/cmake")

# add python directory to PYTHONPATH
include(PathUtils)
python_path_prepend("python")

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS src/*.cpp src-gen/*.cpp)

add_library(@PROJECT_IDENTIFIER@ SHARED ${SRC})
target_include_directories(@PROJECT_IDENTIFIER@ PUBLIC src src-gen)
target_link_libraries(@PROJECT_IDENTIFIER@ PUBLIC Xsmp::Cdk)

# --------------------------------------------------------------------

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    include(Pytest)
    pytest_discover_tests()
endif()
)XSMP_TPL",
                              name));
  }
  {
    const auto file = directory / render("README.md", name);
    writeFileIfMissing(file, render(R"XSMP_TPL(# @PROJECT_NAME@

Project description.

## System Requirements

Check [xsmp-sdk system requirements](https://thalesgroup.github.io/xsmp-sdk/requirements.html).

## How to Build

```bash
cmake -B ./build -DCMAKE_BUILD_TYPE=Release
cmake --build ./build --config Release
```

## How to Test

```bash
cd build && ctest -C Release --output-on-failure
```
)XSMP_TPL",
                                    name));
  }
}

void scaffoldPythonProject(const std::filesystem::path& directory,
                           std::string_view name) {
  {
    const auto file = directory / render("pytest.ini", name);
    writeFileIfMissing(file, render(R"XSMP_TPL(# pytest.ini
[pytest]
testpaths = python
)XSMP_TPL",
                                    name));
  }
  {
    const auto file =
        directory
        / render("python/@PROJECT_IDENTIFIER@/test_@PROJECT_IDENTIFIER@.py",
                 name);
    writeFileIfMissing(file, render(R"XSMP_TPL(import ecss_smp
import xsmp
import @PROJECT_IDENTIFIER@

class Test@PROJECT_IDENTIFIER@(xsmp.unittest.TestCase):
    try:
        sim: @PROJECT_IDENTIFIER@._test_@PROJECT_IDENTIFIER@.Simulator
    except AttributeError:
        pass
    
    def loadAssembly(self, sim: ecss_smp.Smp.ISimulator):
        sim.LoadLibrary("@PROJECT_IDENTIFIER@")
        # Create instances, configuration, and connections here.

    def test_@PROJECT_IDENTIFIER@(self):
        # Add project-specific assertions here.
        pass
)XSMP_TPL",
                                    name));
  }
}

void scaffoldAdocProject(const std::filesystem::path& directory,
                         std::string_view name) {
  {
    const auto file = directory / render("doc/@PROJECT_IDENTIFIER@.adoc", name);
    writeFileIfMissing(file, render(R"XSMP_TPL(:doctype: book
:toc:
:pdf-themesdir: {docdir}

= XSMP document set

This AsciiDoc entry point is intentionally minimal.

Generated documentation files are written to `adoc-gen/` next to the XSMP project root and can be included here as needed.
)XSMP_TPL",
                                    name));
  }
  {
    const auto file = directory / render("doc/themes/default.yml", name);
    writeFileIfMissing(file, render(R"XSMP_TPL(extends: default
page:
  margin: [1.5in, 0.75in]
  numbering:
    start_at: title
base:
  font-family: Helvetica
running_content:
  start_at: title
header:
  height: 1.25in
  border-width: 0.25
  border-color: #DDDDDD
  recto:
    center:
      content: '{testvar}'
    right:
      content: |
        *Date:* {localdatetime} +
        *Page:* {page-number}/{page-count}
  verso:
    center:
      content: $header-recto-center-content
    right:
      content: $header-recto-right-content
footer:
  height: 0.85in
  font-size: 9
  recto:
    left:
      content: ''
    center:
      content: |
        *XSMP* +
        This document has been automatically generated using the Adoc tool from XSMP Modeler. +
        © {localyear}, All Rights Reserved
    right:
      content: ''
  verso:
    left:
      content: $footer-recto-left-content
    center:
      content: $footer-recto-center-content
    right:
      content: $footer-recto-right-content
)XSMP_TPL",
                                    name));
  }
}

}  // namespace xsmp
