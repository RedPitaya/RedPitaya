##
## Unit tests for the client API: GPIOStreamClient against a fake board (ServerNetConfigManager
## on 127.0.0.1, the config port of the API). Built only when BUILD_UNIT_TESTS is ON and run right
## after the build, like the common_lib tests.
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

add_executable(streaming_api_tests test_gpio_client.cpp)

target_include_directories(streaming_api_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/common_lib
    ${CMAKE_BINARY_DIR}/bin/include)

target_link_directories(streaming_api_tests PRIVATE ${CMAKE_BINARY_DIR}/bin/ ${CMAKE_BINARY_DIR}/lib/)
target_link_libraries(streaming_api_tests PRIVATE
    ${CPP_PROJ}
    config_net_lib
    GTest::gtest_main
    pthread)
target_compile_features(streaming_api_tests PRIVATE cxx_std_20)

## The tests share the fixed config port of the API (NET_CONFIG_PORT): one at a time
include(GoogleTest)
gtest_discover_tests(streaming_api_tests PROPERTIES LABELS streaming_api RUN_SERIAL TRUE TIMEOUT 120)
