##
## Unit tests for tdms_lib. Correctness is pinned three ways.
##
##  1. Golden byte arrays, built declaratively with tests/support/spec_builder.h
##     - an encoder that shares no code with TDMS::BinaryStream - and derived by
##     hand from the TDMS 2.0 specification. test_writer_golden.cpp asserts the
##     writer produces exactly those bytes.
##
##  2. test_reader_golden.cpp feeds the SAME hand-derived bytes to Reader and
##     never mentions Writer. That is what stops a bug shared by writer and
##     reader from passing: a round trip through a symmetric misunderstanding
##     succeeds, a pinned byte pattern does not.
##
##  3. compat/compat_nptdms.py checks both directions against npTDMS, an
##     independent reader and writer, when it is installed (pip install npTDMS;
##     TDMS_COMPAT_PYTHON picks the interpreter); otherwise ctest reports it skipped.
##
## test_roundtrip.cpp sits on top of both. The remaining files are regression
## suites for the crashes, hangs and memory defects, plus a deterministic
## mutation sweep over the goldens for robustness. test_value, test_segment_builder,
## test_file_reader and test_api_roundtrip cover the API of tdms.h the same way:
## goldens from spec_builder.h for what SegmentBuilder writes and for what
## FileReader has to read.
##
## Build:  cmake -S . -B build -DBUILD_TESTS=ON [-DTDMS_ENABLE_SANITIZERS=ON]
##         cmake --build build && (cd build && ctest --output-on-failure)
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
else()
    message(STATUS "Using system GTest")
endif()

add_executable(tdms_tests
    test_writer_golden.cpp
    test_reader_golden.cpp
    test_roundtrip.cpp
    test_datatype.cpp
    test_binary_stream.cpp
    test_reader_interleaved.cpp
    test_file_lifecycle.cpp
    test_file_incremental_metadata.cpp
    test_malformed.cpp
    test_widths_and_abi.cpp
    test_print.cpp
    test_timestamp.cpp
    test_api_compat_no_using.cpp
    test_api_compat_std_using.cpp
    test_value.cpp
    test_segment_builder.cpp
    test_file_reader.cpp
    test_api_roundtrip.cpp
    test_file_writer.cpp
)

target_include_directories(tdms_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/support
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(tdms_tests PRIVATE ${PROJECT_NAME} GTest::gtest_main)
target_compile_features(tdms_tests PRIVATE cxx_std_17)

if(TDMS_ENABLE_SANITIZERS)
    target_compile_options(tdms_tests PRIVATE ${_tdms_sanitizer_flags})
    target_link_options(tdms_tests PRIVATE ${_tdms_sanitizer_flags})
endif()

## gtest_discover_tests runs the test binary at build time, which cannot work
## when cross-compiling for the RedPitaya ARM target.
## Labelled so the post-build run can select exactly this library's tests when
## several libraries are configured into one build tree.
## Compatibility with npTDMS, an independent reader and writer of the format: runs when a Python
## with numpy and npTDMS is there (pip install npTDMS), is reported as skipped otherwise.
## TDMS_COMPAT_PYTHON picks the interpreter.
add_executable(tdms_compat_tool compat/tdms_compat_tool.cpp)
target_link_libraries(tdms_compat_tool PRIVATE ${PROJECT_NAME})
target_compile_features(tdms_compat_tool PRIVATE cxx_std_17)
if(NOT CMAKE_CROSSCOMPILING)
    find_package(Python3 COMPONENTS Interpreter QUIET)
    set(TDMS_COMPAT_PYTHON "${Python3_EXECUTABLE}" CACHE FILEPATH "Python with npTDMS for the TDMS compatibility test")
    if(TDMS_COMPAT_PYTHON)
        add_test(NAME tdms_nptdms_compat
                 COMMAND ${TDMS_COMPAT_PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/compat/compat_nptdms.py $<TARGET_FILE:tdms_compat_tool>)
        set_tests_properties(tdms_nptdms_compat PROPERTIES LABELS tdms_lib SKIP_RETURN_CODE 77)
    endif()
endif()
add_dependencies(tdms_tests tdms_compat_tool)

if(CMAKE_CROSSCOMPILING)
    add_test(NAME tdms_tests COMMAND tdms_tests)
    set_tests_properties(tdms_tests PROPERTIES LABELS tdms_lib)
else()
    include(GoogleTest)
    gtest_discover_tests(tdms_tests PROPERTIES LABELS tdms_lib)
endif()
