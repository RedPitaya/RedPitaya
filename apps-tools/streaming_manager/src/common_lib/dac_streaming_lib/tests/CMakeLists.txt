##
## Unit tests for dac_streaming_lib: DACStreamServer playing files through the dummy CGenerator
## of the host build.
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

add_executable(dac_streaming_lib_tests test_dac_stream_server.cpp)

target_include_directories(dac_streaming_lib_tests PRIVATE ${COMMON_LIB_DIR})

target_link_libraries(dac_streaming_lib_tests PRIVATE
    ${PROJECT_NAME}
    board_lib
    uio_lib
    data_lib
    logger_lib
    GTest::gtest_main
    pthread)
target_compile_features(dac_streaming_lib_tests PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(dac_streaming_lib_tests PROPERTIES LABELS dac_streaming_lib TIMEOUT 120)
