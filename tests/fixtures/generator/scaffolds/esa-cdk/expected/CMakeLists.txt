file(GLOB_RECURSE SRC CONFIGURE_DEPENDS src/*.cpp src-gen/*.cpp)

simulus_library(
    MAIN
    SOURCES
        ${SRC}
    LIBRARIES
        esa.ecss.smp.cdk
)
target_include_directories(Mission PUBLIC src src-gen)
