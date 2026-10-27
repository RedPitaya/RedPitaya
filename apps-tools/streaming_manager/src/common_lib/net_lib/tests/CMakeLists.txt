##
## Unit tests for net_lib's streaming data channel, over 127.0.0.1. Built only when
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

add_executable(net_lib_tests test_data_channel.cpp)

target_include_directories(net_lib_tests PRIVATE ${COMMON_LIB_DIR})

## streaming_lib for CStreamingNet, the sender streaming-server uses; data_lib logs through
## logger_lib without linking it.
target_link_libraries(net_lib_tests PRIVATE
    ${PROJECT_NAME}
    streaming_lib
    data_lib
    logger_lib
    GTest::gtest_main
    pthread)
target_compile_features(net_lib_tests PRIVATE cxx_std_20)

if(CMAKE_CROSSCOMPILING)
    add_test(NAME net_lib_tests COMMAND net_lib_tests)
    set_tests_properties(net_lib_tests PROPERTIES LABELS net_lib)
else()
    include(GoogleTest)
    gtest_discover_tests(net_lib_tests PROPERTIES LABELS net_lib)
endif()
