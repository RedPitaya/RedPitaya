##
## Unit tests for rp-ptcc.
##
## C++ side (GoogleTest): exercises the public rp_Ptcc* C API. Split by
## concern, one file per area:
##   - test_api_lifecycle.cpp  init/release, null and range guards, the calls
##                             that must fail before a device is open
##   - test_api_lookup.cpp     status/error text and version lookups, which go
##                             through the embedded interpreter into the
##                             upstream status_messages/error_messages tables
##   - test_api_monitor.cpp    reads against the emulator, SI scaling, the
##                             monitor cache and the background poller
##   - test_api_control.cpp    setpoint/current/cooler/fan plus the failure
##                             paths (silent device, corrupted CRC, LAB_M)
## all compiled into a single "rp_ptcc_tests" binary and registered
## individually with CTest via gtest_discover_tests().
##
## There is no hand-written C++ protocol encoder to check against: the driver
## is a wrapper, so ptcc_emulator.py builds its answers with the upstream
## ptcc-library and the tests assert that what comes out of the C API matches
## what the fixture put in.
##
## Python side: test_rp_ptcc_swig.py exercises the SWIG-generated `rp_ptcc`
## module directly (only built/run when BUILD_PYTHON_MODULE is ON).
##

include(FetchContent)

find_package(GTest QUIET)
if(NOT GTest_FOUND)
    message(STATUS "System GTest not found - fetching GoogleTest via FetchContent")

    get_property(_rp_ptcc_cached_cc CACHE CMAKE_C_COMPILER PROPERTY VALUE)
    get_property(_rp_ptcc_cached_cxx CACHE CMAKE_CXX_COMPILER PROPERTY VALUE)
    if(_rp_ptcc_cached_cc)
        set(CMAKE_C_COMPILER "${_rp_ptcc_cached_cc}")
    endif()
    if(_rp_ptcc_cached_cxx)
        set(CMAKE_CXX_COMPILER "${_rp_ptcc_cached_cxx}")
    endif()

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
else()
    message(STATUS "Using system GTest ${GTest_VERSION}")
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

# The emulator and the bridge both need the upstream package importable. When
# CMake cloned it, point at the clone; otherwise fall back to PYTHONPATH.
set(TEST_PYTHONPATH ${PROJECT_SOURCE_DIR}/python)
if(PTCC_UPSTREAM_PACKAGE)
    get_filename_component(_ptcc_upstream_root ${PTCC_UPSTREAM_PACKAGE} DIRECTORY)
    set(TEST_PYTHONPATH "${TEST_PYTHONPATH}:${_ptcc_upstream_root}")
endif()

add_custom_target(rp_ptcc_run_tests ALL
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS rp_ptcc_tests
    COMMENT "Running rp-ptcc unit tests (ctest) - build fails if any test fails"
    VERBATIM
)
if(BUILD_PYTHON_MODULE AND TARGET rp_ptcc_py)
    add_dependencies(rp_ptcc_run_tests rp_ptcc_py)
endif()

add_executable(rp_ptcc_tests
    test_api_lifecycle.cpp
    test_api_lookup.cpp
    test_api_monitor.cpp
    test_api_control.cpp
    test_api_labm.cpp
)

target_include_directories(rp_ptcc_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${Python3_INCLUDE_DIRS}
)

target_compile_definitions(rp_ptcc_tests PRIVATE
    PTCC_EMULATOR="${CMAKE_CURRENT_SOURCE_DIR}/ptcc_emulator.py"
    PYTHON3_EXECUTABLE="${Python3_EXECUTABLE}"
)

target_link_libraries(rp_ptcc_tests PRIVATE
    ${PROJECT_NAME}-obj
    GTest::gtest_main
    ${Python3_LIBRARIES}
    -lm -lpthread
)

include(GoogleTest)
# Registers every TEST()/TEST_F() with CTest individually, so a decoding
# regression is reported on its own line instead of hiding inside one big
# rp_ptcc_tests pass/fail. The whole suite shares one PYTHONPATH: the tests
# start the emulator as a subprocess and the library imports the bridge into
# its embedded interpreter, and both need the same two directories.
gtest_discover_tests(rp_ptcc_tests
    PROPERTIES ENVIRONMENT "PYTHONPATH=${TEST_PYTHONPATH}"
)

## SWIG / Python wrapper tests -------------------------------------------
## Only registered when the python module is actually being built, since
## they import the compiled `rp_ptcc` extension.
if(BUILD_PYTHON_MODULE AND TARGET rp_ptcc_py)
    add_test(
        NAME swig_wrapper_tests
        COMMAND ${Python3_EXECUTABLE} -m unittest discover -s ${CMAKE_CURRENT_SOURCE_DIR} -p "test_rp_ptcc_swig.py" -v
    )
    # SWIG_ADD_LIBRARY(rp_ptcc_py ...) generates two artifacts that both need
    # to be importable: the compiled extension (_rp_ptcc_py.*, under
    # CMAKE_LIBRARY_OUTPUT_DIRECTORY) and the pure-Python wrapper named after
    # the %module directive in rp_ptcc.i (rp_ptcc.py, in the top-level binary
    # dir). Both go on PYTHONPATH, together with the bridge and the upstream
    # package the extension imports at runtime.
    set_tests_properties(swig_wrapper_tests PROPERTIES
        ENVIRONMENT "PYTHONPATH=$<TARGET_FILE_DIR:rp_ptcc_py>:${CMAKE_BINARY_DIR}:${TEST_PYTHONPATH};PTCC_EMULATOR=${CMAKE_CURRENT_SOURCE_DIR}/ptcc_emulator.py"
    )
endif()
