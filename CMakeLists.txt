cmake_minimum_required(VERSION 3.16)

project(vsm
    VERSION 1.0.0
    DESCRIPTION "Valheim Save Manager - command line save synchronisation"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Some third-party projects declare very old minimum CMake versions;
# newer CMake releases refuse those unless this is set.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

option(VSM_BUILD_TESTS "Build the unit tests" ON)

include(FetchContent)

# ---------------------------------------------------------------- core library
# Every source file is listed explicitly (no file(GLOB)).
set(VSM_SOURCES
    src/Settings.cpp
)

add_library(vsm_core STATIC ${VSM_SOURCES})
target_include_directories(vsm_core PUBLIC include)

if(MSVC)
    target_compile_options(vsm_core PUBLIC /W4)
else()
    target_compile_options(vsm_core PUBLIC -Wall -Wextra -Wpedantic)
endif()

# ------------------------------------------------------------------ executable
# (added together with the CLI)

# ----------------------------------------------------------------------- tests
if(VSM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ------------------------------------------------------------------- housekeeping
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/*
    COMMENT "Cleaning all build files"
)
