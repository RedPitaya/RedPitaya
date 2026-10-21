##
## Unit tests for config_net_lib: the protocol numbers, TriggerStatus, and a server and a
## client talking over 127.0.0.1. Built only when BUILD_UNIT_TESTS is ON and run right after
## the build, like the settings_lib tests.
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

add_executable(config_net_lib_tests
    test_trigger_status.cpp
    test_net_config.cpp
)

target_include_directories(config_net_lib_tests PRIVATE ${COMMON_LIB_DIR})

target_link_libraries(config_net_lib_tests PRIVATE
    ${PROJECT_NAME}
    GTest::gtest_main
    pthread)
target_compile_features(config_net_lib_tests PRIVATE cxx_std_20)

if(CMAKE_CROSSCOMPILING)
    add_test(NAME config_net_lib_tests COMMAND config_net_lib_tests)
    set_tests_properties(config_net_lib_tests PROPERTIES LABELS config_net_lib)
else()
    include(GoogleTest)
    gtest_discover_tests(config_net_lib_tests PROPERTIES LABELS config_net_lib)
endif()
