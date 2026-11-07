if(NOT TARGET GTest::gtest)
  FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG v1.17.0)
  FetchContent_MakeAvailable(googletest)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "WASI")
  # WASI has no mkstemp; these tests do not capture process-wide streams.
  target_compile_definitions(gtest PUBLIC GTEST_HAS_STREAM_REDIRECTION=0)
endif()
file(GLOB_RECURSE _tests CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/tests/*.cpp")
list(FILTER _tests EXCLUDE REGEX "/fixtures/")
list(FILTER _tests EXCLUDE REGEX "/support/TestContribution")
add_executable(xsmp_tests ${_tests})
if(CMAKE_SYSTEM_NAME STREQUAL "WASI" AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_link_options(xsmp_tests PRIVATE --no-wasm-opt)
endif()
target_include_directories(xsmp_tests PRIVATE "${PROJECT_SOURCE_DIR}/tests")
target_link_libraries(xsmp_tests PRIVATE xsmpLsp xsmpTasMdk cursiv::test GTest::gtest pugixml::pugixml)
xsmp_add_extension(xsmp-test-plugin
  SOURCES "${PROJECT_SOURCE_DIR}/tests/support/TestContribution.cpp")
target_compile_definitions(xsmp_tests PRIVATE
  XSMP_BUILTINS_DIR="${PROJECT_SOURCE_DIR}/builtins"
  XSMP_FIXTURES_DIR="${PROJECT_SOURCE_DIR}/tests/fixtures"
  XSMP_EXAMPLE_DIR="${PROJECT_SOURCE_DIR}/example")
include(GoogleTest)
gtest_discover_tests(xsmp_tests DISCOVERY_MODE PRE_TEST
  PROPERTIES ENVIRONMENT "CURSIV_WORKERS=2")
find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND AND (UNIX OR CMAKE_SYSTEM_NAME STREQUAL "WASI"))
  set(_plugin_runner)
  foreach(_part IN LISTS CMAKE_CROSSCOMPILING_EMULATOR)
    list(APPEND _plugin_runner "--runner=${_part}")
  endforeach()
  add_test(NAME xsmp_contribution_process
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tests/integration/contribution_process_test.py
      --cli $<TARGET_FILE:xsmp_cli> --lsp $<TARGET_FILE:xsmp_lsp>
      --plugin $<TARGET_PROPERTY:xsmp-test-plugin,XSMP_EXTENSION_FILE> ${_plugin_runner})
  set_tests_properties(xsmp_contribution_process PROPERTIES TIMEOUT 180)
  add_test(NAME xsmp_lsp_process
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tests/integration/lsp_process_test.py
      ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:xsmp_lsp>)
  set_tests_properties(xsmp_lsp_process PROPERTIES TIMEOUT 45)
  add_test(NAME xsmp_editor_completion
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tests/integration/editor_completion_test.py
      --report ${CMAKE_CURRENT_BINARY_DIR}/editor-completion.json
      -- ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:xsmp_lsp>)
  set_tests_properties(xsmp_editor_completion PROPERTIES TIMEOUT 300)
  add_test(NAME xsmp_editor_navigation
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tests/integration/editor_navigation_test.py
      --report ${CMAKE_CURRENT_BINARY_DIR}/editor-navigation.json
      -- ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:xsmp_lsp>)
  set_tests_properties(xsmp_editor_navigation PROPERTIES TIMEOUT 180)
  add_test(NAME xsmp_production_load
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tests/integration/production_load_test.py
      --lsp $<TARGET_FILE:xsmp_lsp> ${_plugin_runner}
      --report ${CMAKE_CURRENT_BINARY_DIR}/production-load.json)
  set_tests_properties(xsmp_production_load PROPERTIES TIMEOUT 180)
  add_test(NAME xsmp_cli_process
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tests/integration/cli_process_test.py
      ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:xsmp_cli>)
  set_tests_properties(xsmp_cli_process PROPERTIES TIMEOUT 90)
  add_test(NAME xsmp_smp_workspace_process
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tests/integration/smp_workspace_process_test.py
      --lsp $<TARGET_FILE:xsmp_lsp> ${_plugin_runner})
  set_tests_properties(xsmp_smp_workspace_process PROPERTIES TIMEOUT 180)
  find_program(XSMP_NODE_EXECUTABLE NAMES node)
  if(XSMP_NODE_EXECUTABLE)
    add_test(NAME xsmp_cli_contracts
      COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tests/integration/cli_contract_test.py
        --cli $<TARGET_FILE:xsmp_cli> --lsp $<TARGET_FILE:xsmp_lsp>
        --plugin $<TARGET_PROPERTY:xsmp-test-plugin,XSMP_EXTENSION_FILE> --project-version ${PROJECT_VERSION}
        ${_plugin_runner})
    set_tests_properties(xsmp_cli_contracts PROPERTIES TIMEOUT 600)
  endif()
  if(XSMP_NODE_EXECUTABLE)
    add_test(NAME cursiv_wasi_runner
      COMMAND ${XSMP_NODE_EXECUTABLE} --test ${cursiv_SOURCE_DIR}/tools/wasi-run.test.mjs)
    set_tests_properties(cursiv_wasi_runner PROPERTIES TIMEOUT 60)
    add_test(NAME xsmp_wasi_runner
      COMMAND ${CMAKE_COMMAND} -E env "XSMP_TEST_WASI_RUNNER=${cursiv_SOURCE_DIR}/tools/wasi-run.mjs" ${XSMP_NODE_EXECUTABLE} --test ${PROJECT_SOURCE_DIR}/tests/integration/wasi_runner_test.mjs)
    set_tests_properties(xsmp_wasi_runner PROPERTIES TIMEOUT 60)
    add_test(NAME xsmp_tas_contribution_process
      COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tests/integration/tas_contribution_process_test.py
        --lsp $<TARGET_FILE:xsmp_lsp> --package ${PROJECT_SOURCE_DIR}/extensions/tas-mdk
        --handler $<TARGET_PROPERTY:xsmp-tas-mdk-handler,XSMP_EXTENSION_FILE>
        ${_plugin_runner})
    set_tests_properties(xsmp_tas_contribution_process PROPERTIES TIMEOUT 300)
    add_test(NAME xsmp_several_extensions_process
      COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tests/integration/several_extensions_process_test.py
        --cli $<TARGET_FILE:xsmp_cli> --lsp $<TARGET_FILE:xsmp_lsp>
        --plugin $<TARGET_PROPERTY:xsmp-test-plugin,XSMP_EXTENSION_FILE>
        --tas-package ${PROJECT_SOURCE_DIR}/extensions/tas-mdk
        --tas-handler $<TARGET_PROPERTY:xsmp-tas-mdk-handler,XSMP_EXTENSION_FILE>
        ${_plugin_runner})
    set_tests_properties(xsmp_several_extensions_process PROPERTIES TIMEOUT 300)
  endif()
endif()

add_test(NAME embed-builtins
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/embed-builtins
    -P ${PROJECT_SOURCE_DIR}/tests/cmake/EmbedBuiltins_test.cmake)

find_package(Python3 COMPONENTS Interpreter QUIET)
find_program(XSMP_XMLLINT_EXECUTABLE NAMES xmllint)
if(Python3_Interpreter_FOUND AND XSMP_XMLLINT_EXECUTABLE)
  set(_schema_runner)
  foreach(_part IN LISTS CMAKE_CROSSCOMPILING_EMULATOR)
    list(APPEND _schema_runner "--runner=${_part}")
  endforeach()
  add_test(NAME smp-schema
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tests/integration/smp_schema_test.py
      --cli $<TARGET_FILE:xsmp_cli> ${_schema_runner} --xmllint ${XSMP_XMLLINT_EXECUTABLE}
      --xsd ${PROJECT_SOURCE_DIR}/tests/fixtures/workspace/xsd)
endif()

# The real headers of each runtime, for the tests that compile generated code.
set(XSMP_SDK_2020_INCLUDE_DIRS "" CACHE STRING "Optional headers of the XSMP SDK for ECSS SMP 2020")
set(XSMP_SDK_2025_INCLUDE_DIRS "" CACHE STRING "Optional headers of the XSMP SDK for ECSS SMP 2025")
set(XSMP_ESA_CDK_INCLUDE_DIR "" CACHE STRING "Optional headers of the ESA CDK")
find_program(XSMP_CLANG_EXECUTABLE NAMES clang++)
if(Python3_Interpreter_FOUND AND XSMP_CLANG_EXECUTABLE)
  foreach(_profile IN ITEMS xsmp-sdk esa-cdk)
    if(_profile STREQUAL "xsmp-sdk")
      set(_runtime_include "${XSMP_SDK_2025_INCLUDE_DIRS}")
    else()
      set(_runtime_include "${XSMP_ESA_CDK_INCLUDE_DIR}")
    endif()
    if(_runtime_include)
      set(_ir_include)
      foreach(_include IN LISTS _runtime_include)
        list(APPEND _ir_include --include "${_include}")
      endforeach()
      add_test(NAME ${_profile}-ir
        COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tests/integration/cpp_profile_ir_test.py
          --cli $<TARGET_FILE:xsmp_cli> ${_plugin_runner}
          --compiler ${XSMP_CLANG_EXECUTABLE} --profile ${_profile}
          --fixtures ${PROJECT_SOURCE_DIR}/tests/fixtures
          ${_ir_include})
      set_tests_properties(${_profile}-ir PROPERTIES TIMEOUT 600)
    endif()
  endforeach()
endif()

# The goldens stay valid: C++ against the headers given above, compiled by the
# compiler of this build when it targets this machine, XML against the schemas.
# What a machine has no headers for is reported, not failed.
if(CMAKE_CROSSCOMPILING)
  find_program(XSMP_GOLDEN_COMPILER NAMES g++ clang++)
else()
  set(XSMP_GOLDEN_COMPILER ${CMAKE_CXX_COMPILER})
endif()
if(Python3_Interpreter_FOUND AND (XSMP_GOLDEN_COMPILER OR XSMP_XMLLINT_EXECUTABLE))
  set(_golden_arguments)
  if(XSMP_GOLDEN_COMPILER)
    list(APPEND _golden_arguments --compiler ${XSMP_GOLDEN_COMPILER})
    foreach(_include IN LISTS XSMP_SDK_2020_INCLUDE_DIRS)
      list(APPEND _golden_arguments --sdk-2020 "${_include}")
    endforeach()
    foreach(_include IN LISTS XSMP_SDK_2025_INCLUDE_DIRS)
      list(APPEND _golden_arguments --sdk-2025 "${_include}")
    endforeach()
    foreach(_include IN LISTS XSMP_ESA_CDK_INCLUDE_DIR)
      list(APPEND _golden_arguments --esa-cdk "${_include}")
    endforeach()
  endif()
  if(XSMP_XMLLINT_EXECUTABLE)
    list(APPEND _golden_arguments --xmllint ${XSMP_XMLLINT_EXECUTABLE}
      --xsd ${PROJECT_SOURCE_DIR}/tests/fixtures/workspace/xsd)
  endif()
  add_test(NAME golden-validity
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tests/integration/golden_validity_test.py
      --fixtures ${PROJECT_SOURCE_DIR}/tests/fixtures/generator ${_golden_arguments})
  set_tests_properties(golden-validity PROPERTIES TIMEOUT 600 PROCESSORS 8)
endif()

# Expected output is raw, so it does not depend on a clang-format version.
get_directory_property(_process_tests TESTS)
set_property(TEST ${_process_tests} APPEND PROPERTY ENVIRONMENT "XSMP_CLANG_FORMAT=none")
