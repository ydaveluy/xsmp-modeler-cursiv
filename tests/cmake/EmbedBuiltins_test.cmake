# Editing a built-in reaches the generated source on the next build, with no
# manual configure. Run with `cmake -DSOURCE_DIR=... -DWORK_DIR=... -P`.
set(project "${WORK_DIR}/project")
set(build "${WORK_DIR}/build")
file(REMOVE_RECURSE "${WORK_DIR}")
file(WRITE "${project}/CMakeLists.txt" "cmake_minimum_required(VERSION 3.20)
project(embed NONE)
include(\"${SOURCE_DIR}/cmake/EmbedBuiltins.cmake\")
add_custom_target(embed_all ALL)
")

function(check_embedded text)
  execute_process(COMMAND "${CMAKE_COMMAND}" --build "${build}"
                  RESULT_VARIABLE failed OUTPUT_QUIET)
  if(failed)
    message(FATAL_ERROR "The build failed")
  endif()
  file(READ "${build}/generated/Builtins.generated.cpp" generated)
  string(FIND "${generated}" "${text}" at)
  if(at EQUAL -1)
    message(FATAL_ERROR "`${text}` is not in the generated source")
  endif()
endfunction()

file(WRITE "${project}/builtins/a.xsmpcat" "catalogue first\n")
execute_process(COMMAND "${CMAKE_COMMAND}" -S "${project}" -B "${build}"
                RESULT_VARIABLE failed OUTPUT_QUIET)
if(failed)
  message(FATAL_ERROR "The configuration failed")
endif()
check_embedded("catalogue first")

file(WRITE "${project}/builtins/a.xsmpcat" "catalogue second\n")
check_embedded("catalogue second")
