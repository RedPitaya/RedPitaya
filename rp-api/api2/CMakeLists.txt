cmake_minimum_required(VERSION 3.14)
project(rp2)

include(CMakeDoxy.txt)

option(BUILD_SHARED "Builds shared library" ON)
option(BUILD_STATIC "Builds static library" ON)
option(IS_INSTALL "Install library" ON)
option(BUILD_DOC "Build documentation" ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 11)
set(C_STANDARD 11)


if(NOT DEFINED INSTALL_DIR)
    message(WARNING,"Installation path not set. Installation will be skipped")
    set(IS_INSTALL OFF)
endif()

message(STATUS "VERSION=${VERSION}")
message(STATUS "REVISION=${REVISION}")
message(STATUS "Install path ${INSTALL_DIR}")

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

include_directories("include")
include_directories("include/redpitaya")
include_directories("src")

file(GLOB PR_HW_SOURCES "src/*.c")

add_compile_options(-mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC)
add_compile_definitions(ARCH_ARM)
add_compile_options(-std=c11 -Wall -pedantic -Wextra  -DVERSION=${VERSION} -DREVISION=${REVISION} $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os> -ffunction-sections -fdata-sections)

add_library(${PROJECT_NAME}-obj OBJECT ${PR_HW_SOURCES})

if(BUILD_SHARED)
    add_library(${PROJECT_NAME}-shared SHARED)
    set_property(TARGET ${PROJECT_NAME}-shared PROPERTY OUTPUT_NAME ${PROJECT_NAME})
    target_link_options(${PROJECT_NAME}-shared PRIVATE -shared -Wl,--version-script=${CMAKE_SOURCE_DIR}/src/exportmap)
    target_sources(${PROJECT_NAME}-shared PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-obj>)

    if(IS_INSTALL)
        install(TARGETS ${PROJECT_NAME}-shared
            LIBRARY DESTINATION ${INSTALL_DIR}/lib
            ARCHIVE DESTINATION ${INSTALL_DIR}/lib)
        install(DIRECTORY ${CMAKE_SOURCE_DIR}/include
            DESTINATION ${INSTALL_DIR} FILES_MATCHING PATTERN "*.h")
        install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/
            DESTINATION ${INSTALL_DIR}/include/api2 FILES_MATCHING PATTERN "*.h")
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
            DESTINATION ${INSTALL_DIR} FILES_MATCHING PATTERN "*.h")
        install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/
            DESTINATION ${INSTALL_DIR}/include/api2 FILES_MATCHING PATTERN "*.h")
        endif()
endif()

if(BUILD_DOC)
set(DOXY_OUTPUT_LANGUAGE "English")
set(DOXY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doc")
set(DOXY_INPUT ${CMAKE_SOURCE_DIR})
ADD_DOCUMENTATION(doc doc/doxyfile.inc)
endif()

unset(INSTALL_DIR CACHE)