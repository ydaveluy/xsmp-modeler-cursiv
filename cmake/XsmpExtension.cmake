# An extension is a library the server loads and calls directly. It shares the
# memory and the types of the server, so it is built from the same sources, and it
# takes what it uses from the server when it loads.

# A library from `SOURCES`, with `xsmpContributions` for its headers. Its property
# `XSMP_EXTENSION_FILE` names the file. A native Windows server loads no extension,
# so there it is not built: a DLL keeps its own copy of the inline variables that
# `ast_cast` relies on, and an extension runs under the WASI server instead.
function(xsmp_add_extension name)
  cmake_parse_arguments(PARSE_ARGV 1 ARG "" "" "SOURCES;INCLUDES")
  if(WIN32)
    return()
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "WASI")
    set(_support "${PROJECT_SOURCE_DIR}/cmake/extension")
    set(_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.wasm")
    add_library(${name}_objects OBJECT ${ARG_SOURCES} "${_support}/WasiLibrary.cpp")
    target_include_directories(${name}_objects PRIVATE ${ARG_INCLUDES})
    target_link_libraries(${name}_objects PRIVATE $<COMPILE_ONLY:xsmpContributions>)
    add_dependencies(${name}_objects xsmpContributions)
    # Hidden is the default of WASI. A library must see the data of the server, as
    # `ast_type_tag`, through the table a load fills.
    target_compile_options(${name}_objects PRIVATE -fPIC -fvisibility=default
      "SHELL:-include ${_support}/wasi-errno.h")
    set(_shared_memory)
    if(CURSIV_WASI_THREADS)
      set(_shared_memory -pthread -Wl,--shared-memory)
    endif()
    add_custom_command(OUTPUT "${_file}"
      COMMAND ${CMAKE_CXX_COMPILER} --target=${CMAKE_CXX_COMPILER_TARGET}
        --sysroot=${CMAKE_SYSROOT} -fPIC -shared -nostdlib ${_shared_memory}
        -Wno-unused-command-line-argument
        -Wl,--experimental-pic,--import-memory,--no-entry
        -Wl,--unresolved-symbols=import-dynamic
        -Wl,--export-if-defined=__wasm_init_tls,--export-if-defined=__tls_size
        -Wl,--export-if-defined=__tls_align,--export-if-defined=__wasm_call_ctors
        $<TARGET_OBJECTS:${name}_objects> -o "${_file}"
      DEPENDS ${name}_objects $<TARGET_OBJECTS:${name}_objects>
      COMMAND_EXPAND_LISTS VERBATIM)
    add_custom_target(${name} ALL DEPENDS "${_file}")
    set_property(GLOBAL APPEND PROPERTY XSMP_EXTENSION_LIBRARIES "${_file}")
  else()
    set(_file "${CMAKE_CURRENT_BINARY_DIR}/${name}${CMAKE_SHARED_MODULE_SUFFIX}")
    add_library(${name} MODULE ${ARG_SOURCES})
    target_include_directories(${name} PRIVATE ${ARG_INCLUDES})
    target_link_libraries(${name} PRIVATE $<COMPILE_ONLY:xsmpContributions>)
    add_dependencies(${name} xsmpContributions)
    set_target_properties(${name} PROPERTIES PREFIX "")
    if(APPLE)
      target_link_options(${name} PRIVATE "LINKER:-undefined,dynamic_lookup")
    endif()
  endif()
  set_target_properties(${name} PROPERTIES XSMP_EXTENSION_FILE "${_file}")
endfunction()

# Lets each target load extensions, once all of them are known.
function(xsmp_link_extensions)
  if(WIN32)
    return()
  endif()
  if(NOT CMAKE_SYSTEM_NAME STREQUAL "WASI")
    set_target_properties(${ARGN} PROPERTIES ENABLE_EXPORTS ON)
    return()
  endif()
  get_property(_libraries GLOBAL PROPERTY XSMP_EXTENSION_LIBRARIES)
  find_program(XSMP_BUILD_NODE NAMES node REQUIRED)
  set(_options "${PROJECT_BINARY_DIR}/extension-exports.rsp")
  set(_tool "${PROJECT_SOURCE_DIR}/tools/extension-exports.mjs")
  add_custom_command(OUTPUT "${_options}"
    COMMAND ${XSMP_BUILD_NODE} "${_tool}" "${_options}" ${_libraries}
    DEPENDS "${_tool}" ${_libraries} VERBATIM)
  add_custom_target(xsmp-extension-exports DEPENDS "${_options}")
  foreach(_target IN LISTS ARGN)
    add_dependencies(${_target} xsmp-extension-exports)
    set_property(TARGET ${_target} APPEND PROPERTY LINK_DEPENDS "${_options}")
    target_link_options(${_target} PRIVATE
      "LINKER:--export-table,--growable-table,--export=__stack_pointer"
      "LINKER:--export=aligned_alloc,--export=__cpp_exception"
      "LINKER:--export=xsmp_errno_location" "@${_options}")
  endforeach()
endfunction()
