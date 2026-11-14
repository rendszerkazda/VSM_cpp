FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# Every test file is listed explicitly.
add_executable(vsm_tests
    SettingsTest.cpp
)
target_link_libraries(vsm_tests PRIVATE vsm_core GTest::gtest_main)

gtest_discover_tests(vsm_tests)
