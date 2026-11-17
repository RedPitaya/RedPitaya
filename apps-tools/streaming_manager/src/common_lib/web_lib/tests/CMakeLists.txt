##
## Unit tests for web_lib: a raw TCP client speaks WebSocket to the server. Built only when
## BUILD_UNIT_TESTS is ON and run right after the build, like the settings_lib tests.
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

add_executable(web_lib_tests test_websocket_server.cpp)

target_include_directories(web_lib_tests PRIVATE ${COMMON_LIB_DIR})

target_link_libraries(web_lib_tests PRIVATE
    ${PROJECT_NAME}
    GTest::gtest_main
    pthread)
target_compile_features(web_lib_tests PRIVATE cxx_std_20)

if(CMAKE_CROSSCOMPILING)
    add_test(NAME web_lib_tests COMMAND web_lib_tests)
    set_tests_properties(web_lib_tests PROPERTIES LABELS web_lib)
else()
    include(GoogleTest)
    gtest_discover_tests(web_lib_tests PROPERTIES LABELS web_lib)
endif()
