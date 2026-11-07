cmake_minimum_required(VERSION 3.30)
project(xsmp VERSION 0.1.0 LANGUAGES CXX)

include(FetchContent)
# Cursiv is downloaded from its main branch. To build against a local copy of
# Cursiv instead, configure with `-DCURSIV_SOURCE_DIR=<path>`.
if(CURSIV_SOURCE_DIR)
  FetchContent_Declare(cursiv SOURCE_DIR ${CURSIV_SOURCE_DIR})
else()
  FetchContent_Declare(cursiv
      GIT_REPOSITORY https://github.com/ydaveluy/cursiv.git
      GIT_TAG        main)
endif()
FetchContent_MakeAvailable(cursiv)

# WebAssembly (WASI) builds run their tests with Node.js.
if(CMAKE_SYSTEM_NAME STREQUAL "WASI" AND CURSIV_NODE)
  set(CMAKE_CROSSCOMPILING_EMULATOR ${CURSIV_NODE} --no-warnings
      "${cursiv_SOURCE_DIR}/tools/wasi-run.mjs"
      --host-module "${PROJECT_SOURCE_DIR}/tools/wasi-host.mjs")
endif()

# SMP XML import and export use the MIT-licensed pugixml 1.15 release.
FetchContent_Declare(pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG        ee86beb30e4973f5feffe3ce63bfa4fbadf72f38)
FetchContent_MakeAvailable(pugixml)

include(cmake/XsmpExtension.cmake)

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
xsmp_link_extensions(xsmp_cli xsmp_lsp)
