##
## Unit tests for reader_lib.
##
## test_metadata_cursor.cpp drives CReaderController through its public
## interface (Create + getBufferPrepared) over TDMS files built with the real
## tdms_lib writer, so the metadata cursor is exercised end to end rather than
## through a copy of its logic.
##
## Built only when BUILD_TESTS is ON, and then run automatically right after the
## library builds - see the rp_reader_lib_run_tests target in ../CMakeLists.txt.
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

add_executable(reader_lib_tests
    test_metadata_cursor.cpp
    test_sources.cpp
)

target_include_directories(reader_lib_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
    ${COMMON_LIB_DIR}
)

target_link_directories(reader_lib_tests
    PRIVATE
    ${CMAKE_BINARY_DIR}/bin/
    ${CMAKE_BINARY_DIR}/lib/
    )

## reader_lib links wav_lib publicly, but reader_controller.cpp also pulls in
## settings_lib (CStreamSettings), data_lib (memcpy_neon) and logger_lib
## (acprintf/ERROR_LOG), which reach the test binary only if named here.
target_link_libraries(reader_lib_tests PRIVATE
    ${PROJECT_NAME}
    tdms_lib
    settings_lib
    data_lib
    logger_lib
    GTest::gtest_main
    pthread)
target_compile_features(reader_lib_tests PRIVATE cxx_std_20)

## Labelled so the post-build run can select exactly this library's tests when
## several libraries are configured into one build tree.
if(CMAKE_CROSSCOMPILING)
    ## gtest_discover_tests runs the test binary on the host at build time, which
    ## cannot work for an ARM binary.
    add_test(NAME reader_lib_tests COMMAND reader_lib_tests)
    set_tests_properties(reader_lib_tests PROPERTIES LABELS reader_lib)
else()
    include(GoogleTest)
    gtest_discover_tests(reader_lib_tests PROPERTIES LABELS reader_lib)
endif()
