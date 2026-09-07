cmake_minimum_required(VERSION 3.14)
project(rp-ptcc)


option(BUILD_SHARED "Builds shared library" ON)
option(BUILD_STATIC "Builds static library" ON)
option(BUILD_PYTHON_MODULE "Builds python module" ON)
option(BUILD_UNIT_TESTS "Build unit tests (GoogleTest C++ suite + SWIG python suite)" OFF)

option(IS_INSTALL "Install library" ON)

# The library wraps the upstream VIGO Python package; this clones it so it can
# be installed next to the bridge module.
option(PTCC_FETCH_UPSTREAM "Clone ptcc-library from GitLab" ON)

if(NOT DEFINED PTCC_UPSTREAM_URL)
    set(PTCC_UPSTREAM_URL "https://gitlab.com/vigophotonics/ptcc-library.git")
endif()

if(NOT DEFINED PTCC_UPSTREAM_TAG)
    set(PTCC_UPSTREAM_TAG "v0.2.1")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 20)
set(C_STANDARD 20)

if(NOT DEFINED INSTALL_DIR)
    message(WARNING "Installation path not set. Installation will be skipped")
    set(IS_INSTALL OFF)
    set(INSTALL_DIR ${CMAKE_BINARY_DIR}/install)
endif()

if(NOT DEFINED VERSION)
  set(VERSION 0.00-0000)
endif()

if(NOT DEFINED REVISION)
  set(REVISION devbuild)
endif()

if(NOT DEFINED BUILD_NUMBER)
  set(BUILD_NUMBER devbuild)
endif()

message(STATUS "Project=${PROJECT_NAME}")
message(STATUS "VERSION=${VERSION}")
message(STATUS "REVISION=${REVISION}")
message(STATUS "BUILD_NUMBER=${BUILD_NUMBER}")
message(STATUS "LINUX_VER=${LINUX_VER}")
message(STATUS "Is install ${IS_INSTALL}")
message(STATUS "Install path ${INSTALL_DIR}")
message(STATUS "Run tests ${BUILD_UNIT_TESTS}")

message(STATUS "Compiler C path: ${CMAKE_C_COMPILER}")
message(STATUS "Compiler C ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "Compiler C version: ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "Compiler C is part: ${CMAKE_COMPILER_IS_GNUC}")

message(STATUS "Compiler C++ path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler C++ ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler C++version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Compiler C++ is part: ${CMAKE_COMPILER_IS_GNUCXX}")

list(APPEND r_paths
    /opt/redpitaya/lib
    /opt/redpitaya/lib/web
)

set(CMAKE_INSTALL_RPATH ${r_paths})

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FetchPtccLibrary.cmake)

# libpython is linked directly: the protocol lives in the upstream Python
# package and is called through the C API.
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
message(STATUS "Python ${Python3_VERSION} at ${Python3_EXECUTABLE}")
message(STATUS "Python libraries: ${Python3_LIBRARIES}")

if(Python3_VERSION VERSION_LESS 3.12)
    message(FATAL_ERROR "The upstream ptcc-library uses typing.override and needs Python 3.12 or newer, "
                        "found ${Python3_VERSION}")
endif()

# A cross build must link the target's libpython, not the build host's. Point
# CMake at the sysroot copy with -DPython3_ROOT_DIR=<sysroot>/usr.
if(CMAKE_CROSSCOMPILING AND CMAKE_SYSROOT AND NOT Python3_LIBRARIES MATCHES "^${CMAKE_SYSROOT}")
    message(WARNING "Cross compiling but Python3_LIBRARIES (${Python3_LIBRARIES}) is outside the sysroot "
                    "${CMAKE_SYSROOT}. Set Python3_ROOT_DIR to the target Python or build on the board.")
endif()

# Directory the bridge adds to sys.path at runtime.
if(NOT DEFINED PTCC_PYTHON_DIR)
    set(PTCC_PYTHON_DIR ${INSTALL_DIR}/lib/python)
endif()

include_directories("include")
include_directories("src")
include_directories(${INSTALL_DIR}/include)
include_directories(${INSTALL_DIR}/include/common)

file(GLOB PR_HW_SOURCES "src/*.cpp")

# ARM switches for the Zynq target only; host builds are for unit tests.
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|aarch64)")
    add_compile_options(-mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC)
    add_compile_definitions(ARCH_ARM)
else()
    add_compile_options(-fPIC)
    message(STATUS "Not an ARM target, skipping cortex-a9 flags")
endif()

add_compile_options($<$<CONFIG:Debug>:-DTRACE_ENABLE>)
add_compile_options(-Wall -Wno-psabi -Wextra -Wpedantic -DVERSION=${VERSION} -DREVISION=${REVISION} $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os> -ffunction-sections -fdata-sections)

add_library(${PROJECT_NAME}-obj OBJECT ${PR_HW_SOURCES})
target_include_directories(${PROJECT_NAME}-obj PUBLIC ${Python3_INCLUDE_DIRS})
target_compile_definitions(${PROJECT_NAME}-obj PUBLIC PTCC_PYTHON_DIR="${PTCC_PYTHON_DIR}")

link_directories(${INSTALL_DIR}/lib)
link_directories(${INSTALL_DIR}/lib/python)

if(BUILD_SHARED)
    if (BUILD_PYTHON_MODULE)
        find_package(SWIG REQUIRED)
        INCLUDE(${SWIG_USE_FILE})

        include_directories(${Python3_INCLUDE_DIRS})

        set_property(SOURCE src/rp_ptcc.i PROPERTY CPLUSPLUS ON)
        set(CMAKE_SWIG_FLAGS -threads -O)

        SWIG_ADD_LIBRARY(rp_ptcc_py LANGUAGE python SOURCES src/rp_ptcc.i ${PR_HW_SOURCES})
        target_include_directories(rp_ptcc_py PRIVATE ${Python3_INCLUDE_DIRS})
        target_compile_definitions(rp_ptcc_py PRIVATE PTCC_PYTHON_DIR="${PTCC_PYTHON_DIR}")
        SWIG_LINK_LIBRARIES(rp_ptcc_py ${Python3_LIBRARIES} pthread)
    endif()

    add_library(${PROJECT_NAME}-shared SHARED)
    set_property(TARGET ${PROJECT_NAME}-shared PROPERTY OUTPUT_NAME ${PROJECT_NAME})
    target_link_options(${PROJECT_NAME}-shared PRIVATE -shared -Wl,--version-script=${CMAKE_SOURCE_DIR}/src/exportmap)
    target_sources(${PROJECT_NAME}-shared PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-obj>)
    target_link_libraries(${PROJECT_NAME}-shared ${Python3_LIBRARIES} pthread)

    if(IS_INSTALL)
        install(TARGETS ${PROJECT_NAME}-shared
            LIBRARY DESTINATION ${INSTALL_DIR}/lib
            ARCHIVE DESTINATION ${INSTALL_DIR}/lib)
        install(DIRECTORY ${CMAKE_SOURCE_DIR}/include
            DESTINATION ${INSTALL_DIR} PATTERN "*.h")

        if (BUILD_PYTHON_MODULE)
            install(FILES $<TARGET_PROPERTY:rp_ptcc_py,SWIG_SUPPORT_FILES> DESTINATION ${INSTALL_DIR}/lib/python)
            install(TARGETS rp_ptcc_py
                LIBRARY DESTINATION ${INSTALL_DIR}/lib/python
                ARCHIVE DESTINATION ${INSTALL_DIR}/lib/python)
            install(FILES tests/rp_ptcc_test.py
                DESTINATION ${INSTALL_DIR}/lib/python PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                GROUP_EXECUTE GROUP_READ WORLD_READ WORLD_WRITE WORLD_EXECUTE)
        endif()
    endif()
endif()


if(BUILD_STATIC)
    add_library(${PROJECT_NAME}-static STATIC)
    set_property(TARGET ${PROJECT_NAME}-static PROPERTY OUTPUT_NAME ${PROJECT_NAME})
    target_sources(${PROJECT_NAME}-static PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-obj>)

    if(IS_INSTALL)
        install(TARGETS ${PROJECT_NAME}-static
            LIBRARY DESTINATION ${INSTALL_DIR}/lib
            ARCHIVE DESTINATION ${INSTALL_DIR}/lib)
        install(DIRECTORY ${CMAKE_SOURCE_DIR}/include
            DESTINATION ${INSTALL_DIR} PATTERN "*.h")
    endif()
endif()

# The bridge module and the upstream package it wraps.
if(IS_INSTALL)
    install(FILES ${CMAKE_SOURCE_DIR}/python/rp_ptcc_bridge.py
        DESTINATION ${INSTALL_DIR}/lib/python)

    if(PTCC_UPSTREAM_PACKAGE)
        install(DIRECTORY ${PTCC_UPSTREAM_PACKAGE}
            DESTINATION ${INSTALL_DIR}/lib/python
            PATTERN "__pycache__" EXCLUDE)
    endif()
endif()

if(BUILD_UNIT_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

unset(INSTALL_DIR CACHE)
