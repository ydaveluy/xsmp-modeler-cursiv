cmake_minimum_required(VERSION 3.14)

project(
    project-name
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

add_library(project_name SHARED ${SRC})
target_include_directories(project_name PUBLIC src src-gen)
target_link_libraries(project_name PUBLIC Xsmp::Cdk)

# --------------------------------------------------------------------

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    include(Pytest)
    pytest_discover_tests()
endif()
