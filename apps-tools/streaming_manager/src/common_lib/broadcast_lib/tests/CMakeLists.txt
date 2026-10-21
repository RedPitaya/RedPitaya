##
## Unit tests for broadcast_lib over the loopback broadcast address. Built only when
## BUILD_UNIT_TESTS is ON and run right after the build, like the settings_lib tests.
##

find_package(GTest QUIET)
if(NOT TARGET GTest::gtest_main)
    message(STATUS "System GTest not found - fetching GoogleTest via FetchContent")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
        GIT_SHALLOW    TRUE
    )
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

add_executable(broadcast_lib_tests test_broadcast.cpp)

target_include_directories(broadcast_lib_tests PRIVATE ${COMMON_LIB_DIR})

target_link_libraries(broadcast_lib_tests PRIVATE
    ${PROJECT_NAME}
    GTest::gtest_main
    pthread)
target_compile_features(broadcast_lib_tests PRIVATE cxx_std_20)

if(CMAKE_CROSSCOMPILING)
    add_test(NAME broadcast_lib_tests COMMAND broadcast_lib_tests)
    set_tests_properties(broadcast_lib_tests PROPERTIES LABELS broadcast_lib)
else()
    include(GoogleTest)
    gtest_discover_tests(broadcast_lib_tests PROPERTIES LABELS broadcast_lib)
endif()
