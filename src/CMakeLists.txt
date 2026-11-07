# One library per package, exposing only its own include root.
function(xsmp_add_package name folder)
  file(GLOB_RECURSE sources CONFIGURE_DEPENDS
       ${CMAKE_CURRENT_SOURCE_DIR}/${folder}/*.cpp)
  add_library(${name} ${sources})
  target_include_directories(${name} PUBLIC ${folder})
endfunction()

# Core: the six languages. Generates their parsers from the grammars under core/,
# again when a grammar changes, then builds the services and checks under core/.
# Every other package links it.
cursiv_generate_language(ID xsmpcat
  GRAMMAR xsmp/XsmpcatGrammar.hpp NAMESPACE xsmp::Xsmpcat
  EXTENSIONS .xsmpcat OUT xsmp/generated)
cursiv_generate_language(ID xsmpproject
  GRAMMAR xsmp/XsmpprojectGrammar.hpp NAMESPACE xsmp::Xsmpproject
  EXTENSIONS .xsmpprofile .xsmptool FILE_NAMES xsmp.project OUT xsmp/generated)
foreach(language IN ITEMS xsmpasb xsmpcfg xsmplnk xsmpsed)
  string(SUBSTRING "${language}" 1 -1 suffix)
  cursiv_generate_language(ID ${language}
    GRAMMAR xsmp/X${suffix}Grammar.hpp NAMESPACE xsmp::X${suffix}
    EXTENSIONS .${language} OUT xsmp/generated)
endforeach()

include(${PROJECT_SOURCE_DIR}/cmake/EmbedBuiltins.cmake)
xsmp_add_package(xsmpCore core)
target_sources(xsmpCore PRIVATE ${XSMP_BUILTINS_GENERATED})
target_link_libraries(xsmpCore PUBLIC
  xsmpcatGenerated xsmpprojectGenerated xsmpasbGenerated xsmpcfgGenerated
  xsmplnkGenerated xsmpsedGenerated)

# Generator: the C++ generation that the profiles and the tools share.
xsmp_add_package(xsmpGenerator generator)
target_link_libraries(xsmpGenerator PUBLIC xsmpCore)

# Tools: the SMP, Python and AsciiDoc tools.
xsmp_add_package(xsmpTools tools)
target_link_libraries(xsmpTools PUBLIC xsmpGenerator PRIVATE pugixml::pugixml)

# Profiles: the XSMP-SDK and ESA-CDK profiles.
xsmp_add_package(xsmpProfiles profiles)
target_link_libraries(xsmpProfiles PUBLIC xsmpGenerator)

# Contributions: the registry of profiles and tools, and the API of an extension.
xsmp_add_package(xsmpContributions contributions)
target_link_libraries(xsmpContributions PUBLIC
  xsmpTools xsmpProfiles cursiv::cli cursiv::lsp)
target_link_libraries(xsmpContributions PUBLIC ${CMAKE_DL_LIBS})
target_compile_definitions(xsmpContributions PUBLIC
  XSMP_EXTENSION_ABI="${PROJECT_VERSION}")

# CLI: the command line. cli.cpp starts the executable, which has TAS-MDK built in.
add_executable(xsmp_cli cli.cpp)
target_link_libraries(xsmp_cli PRIVATE xsmpTasMdk)
target_compile_definitions(xsmp_cli PRIVATE XSMP_VERSION="${PROJECT_VERSION}")

# LSP: the language server. lsp.cpp starts the executable; the code under lsp/
# builds into a library that the tests and the benchmark can link.
xsmp_add_package(xsmpLsp lsp)
target_link_libraries(xsmpLsp PUBLIC xsmpContributions)
add_executable(xsmp_lsp lsp.cpp)
target_link_libraries(xsmp_lsp PRIVATE xsmpLsp)

# Benchmark: bench.cpp measures the languages through the LSP library.
cursiv_add_benchmark(xsmp SOURCES bench.cpp)

# TAS-MDK: a profile that lives in its own extension, which the CLI links too.
add_subdirectory(${PROJECT_SOURCE_DIR}/extensions/tas-mdk ${PROJECT_BINARY_DIR}/extensions/tas-mdk)

# VS Code: a WASI build copies the language server into vscode/server/. Cursiv's
# runner and the XSMP host module stand beside it, in development too.
file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/vscode/server")
configure_file("${cursiv_SOURCE_DIR}/tools/wasi-run.mjs"
  "${PROJECT_SOURCE_DIR}/vscode/server/wasi-run.mjs" COPYONLY)
configure_file("${PROJECT_SOURCE_DIR}/tools/wasi-host.mjs"
  "${PROJECT_SOURCE_DIR}/vscode/server/wasi-host.mjs" COPYONLY)
cursiv_package_language(xsmp)
