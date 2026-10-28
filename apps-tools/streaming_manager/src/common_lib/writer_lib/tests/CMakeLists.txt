##
## Unit tests for writer_lib: the chunks the file thread writes, its queue and thread, the
## segments of the BIN, TDMS and WAV formats, and CStreamingFile writing them end to end.
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

add_executable(writer_lib_tests test_file_queue.cpp test_formats.cpp)

target_include_directories(writer_lib_tests PRIVATE ${COMMON_LIB_DIR})

## streaming_lib for CStreamingFile and wav_lib for the WAV segments; the packs come from
## data_lib through the helpers of its tests (data_lib/tests/support.h).
target_link_libraries(writer_lib_tests PRIVATE
    streaming_lib
    wav_lib
    ${PROJECT_NAME}
    data_lib
    logger_lib
    GTest::gtest_main
    pthread)
target_compile_features(writer_lib_tests PRIVATE cxx_std_20)

if(CMAKE_CROSSCOMPILING)
    add_test(NAME writer_lib_tests COMMAND writer_lib_tests)
    set_tests_properties(writer_lib_tests PROPERTIES LABELS writer_lib)
else()
    include(GoogleTest)
    gtest_discover_tests(writer_lib_tests PROPERTIES LABELS writer_lib)
endif()
