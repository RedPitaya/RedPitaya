##
## Unit tests for streaming_lib: CStreamingFPGA filling the buffer header from the scope
## (the dummy COscilloscope of the host build stands for the FPGA).
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

add_executable(streaming_lib_tests test_streaming_fpga.cpp test_adc_stream_server.cpp)

target_include_directories(streaming_lib_tests PRIVATE ${COMMON_LIB_DIR})

target_link_libraries(streaming_lib_tests PRIVATE
    ${PROJECT_NAME}
    board_lib
    uio_lib
    data_lib
    logger_lib
    GTest::gtest_main
    pthread)
target_compile_features(streaming_lib_tests PRIVATE cxx_std_20)

include(GoogleTest)
## The blocks take 5% of the host RAM: a fixed limit keeps the results the same on every host
gtest_discover_tests(streaming_lib_tests PROPERTIES LABELS streaming_lib ENVIRONMENT RP_STREAMING_RAM_LIMIT=128M)
