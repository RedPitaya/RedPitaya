##
## Unit tests for converter_lib: a BIN stream file converted on the host.
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

add_executable(converter_lib_tests test_converter.cpp)

target_include_directories(converter_lib_tests PRIVATE ${COMMON_LIB_DIR})

target_link_libraries(converter_lib_tests PRIVATE
    ${PROJECT_NAME}
    bin_lib
    tdms_lib
    logger_lib
    GTest::gtest_main
    pthread)
target_compile_features(converter_lib_tests PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(converter_lib_tests PROPERTIES LABELS converter_lib TIMEOUT 120)
