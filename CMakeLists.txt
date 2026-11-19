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
    src/FileUtil.cpp
    src/LocalFolderStorage.cpp
    src/GzipCompressor.cpp
    src/Config.cpp
    src/PathProvider.cpp
    src/CurlProcessHttpClient.cpp
    src/SupabaseStorage.cpp
    src/Events.cpp
    src/ActivityLog.cpp
)

# nlohmann/json (header only)
FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

# zlib: use the system one if present, otherwise download and build it.
find_package(ZLIB QUIET)
if(NOT ZLIB_FOUND)
    message(STATUS "System zlib not found, fetching zlib 1.3.1")
    FetchContent_Declare(zlib
        URL https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.gz
    )
    FetchContent_MakeAvailable(zlib)
    add_library(vsm_zlib INTERFACE)
    target_link_libraries(vsm_zlib INTERFACE zlibstatic)
    target_include_directories(vsm_zlib INTERFACE ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
else()
    add_library(vsm_zlib INTERFACE)
    target_link_libraries(vsm_zlib INTERFACE ZLIB::ZLIB)
endif()

add_library(vsm_core STATIC ${VSM_SOURCES})
target_include_directories(vsm_core PUBLIC include)
target_link_libraries(vsm_core PUBLIC vsm_zlib nlohmann_json::nlohmann_json)

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
