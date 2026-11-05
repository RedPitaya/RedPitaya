##
## Unit tests for gpio_streaming_lib: GPIOStreamServer on the dummy CGpio of the host build,
## with the clients of its data ports over TCP on localhost.
## Built only when BUILD_UNIT_TESTS is ON and run right after the build.
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

add_executable(gpio_streaming_lib_tests test_gpio_stream_server.cpp)

target_include_directories(gpio_streaming_lib_tests PRIVATE ${COMMON_LIB_DIR})

target_link_libraries(gpio_streaming_lib_tests PRIVATE
    ${PROJECT_NAME}
    board_lib
    uio_lib
    data_lib
    net_lib
    logger_lib
    GTest::gtest_main
    pthread)
target_compile_features(gpio_streaming_lib_tests PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(gpio_streaming_lib_tests PROPERTIES LABELS gpio_streaming_lib TIMEOUT 120)
