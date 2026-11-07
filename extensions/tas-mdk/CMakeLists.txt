file(GLOB _tas CONFIGURE_DEPENDS
     ${CMAKE_CURRENT_SOURCE_DIR}/src/xsmp/tas-mdk/*.cpp)
add_library(xsmpTasMdk ${_tas})
target_include_directories(xsmpTasMdk PUBLIC src)
target_link_libraries(xsmpTasMdk PUBLIC xsmpContributions)

xsmp_add_extension(xsmp-tas-mdk-handler SOURCES ${_tas} src/main.cpp
  INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(CMAKE_SYSTEM_NAME STREQUAL "WASI")
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/server")
  add_custom_command(TARGET xsmp-tas-mdk-handler POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_PROPERTY:xsmp-tas-mdk-handler,XSMP_EXTENSION_FILE>"
      "${CMAKE_CURRENT_SOURCE_DIR}/server/tas-mdk.wasm"
    VERBATIM)
endif()
