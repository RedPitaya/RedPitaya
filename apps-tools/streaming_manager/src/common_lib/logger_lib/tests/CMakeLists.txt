##
## Unit tests for logger_lib's CFileLogger: the per buffer file (<name>.lost.txt) and
## the report (<name>.txt) written into a temporary directory. Built only when
## BUILD_UNIT_TESTS is ON and run right after the build, like the data_lib tests.
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

add_executable(logger_lib_tests test_file_logger.cpp)

target_include_directories(logger_lib_tests PRIVATE ${COMMON_LIB_DIR})

## The buffers come from data_lib, through the helpers of its tests (data_lib/tests/support.h).
target_link_libraries(logger_lib_tests PRIVATE
    ${PROJECT_NAME}
    data_lib
    settings_lib
    GTest::gtest_main
    pthread)
target_compile_features(logger_lib_tests PRIVATE cxx_std_20)

if(CMAKE_CROSSCOMPILING)
    add_test(NAME logger_lib_tests COMMAND logger_lib_tests)
    set_tests_properties(logger_lib_tests PROPERTIES LABELS logger_lib)
else()
    include(GoogleTest)
    gtest_discover_tests(logger_lib_tests PROPERTIES LABELS logger_lib)
endif()
