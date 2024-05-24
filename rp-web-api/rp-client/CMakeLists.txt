cmake_minimum_required(VERSION 3.22)
project(rp-client)

option(BUILD_SHARED "Builds shared library" ON)
option(BUILD_STATIC "Builds static library" ON)

option(IS_INSTALL "Install library" ON)
option(BUILD_DOC "Build documentation" ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 17)


if(NOT DEFINED INSTALL_DIR)
    message(WARNING,"Installation path not set. Installation will be skipped")
    set(IS_INSTALL OFF)
endif()

if(NOT DEFINED DEBUG_REG)
  set(DEBUG_REG OFF)
endif()


if(NOT DEFINED VERSION)
  set(VERSION 0.00-0000)
endif()

if(NOT DEFINED REVISION)
  set(REVISION devbuild)
endif()


message(STATUS "VERSION=${VERSION}")
message(STATUS "REVISION=${REVISION}")
message(STATUS "Is install ${IS_INSTALL}")
message(STATUS "Install path ${INSTALL_DIR}")

message(STATUS "Build shared ${BUILD_SHARED}")
message(STATUS "Build static ${BUILD_STATIC}")

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

list(APPEND src
    ${CMAKE_SOURCE_DIR}/src/rp_client.cpp
)

list(APPEND header
    ${CMAKE_SOURCE_DIR}/src/rp_client.h
)

add_library(${PROJECT_NAME}-obj OBJECT ${src})

target_include_directories(${PROJECT_NAME}-obj PUBLIC ${INSTALL_DIR}/rp_sdk)
target_include_directories(${PROJECT_NAME}-obj PUBLIC ${INSTALL_DIR}/rp_sdk/libjson)
target_include_directories(${PROJECT_NAME}-obj PUBLIC ${INSTALL_DIR}/include)
target_include_directories(${PROJECT_NAME}-obj PUBLIC ${INSTALL_DIR}/include/api250-12)
target_include_directories(${PROJECT_NAME}-obj PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME}-obj PUBLIC ${CMAKE_SOURCE_DIR}/src)


if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    target_compile_options(${PROJECT_NAME}-obj
        PRIVATE -mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC)
    target_compile_definitions(${PROJECT_NAME}-obj
        PRIVATE ARCH_ARM)
endif()

target_compile_options(${PROJECT_NAME}-obj PRIVATE -Wall -Wno-reorder -Wextra -Wno-unused-parameter -DVERSION=${VERSION} -DREVISION=${REVISION} $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os> -ffunction-sections -fdata-sections)


if(BUILD_SHARED)

    add_library(${PROJECT_NAME}-shared SHARED)
    set_property(TARGET ${PROJECT_NAME}-shared PROPERTY OUTPUT_NAME ${PROJECT_NAME})
    target_sources(${PROJECT_NAME}-shared PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-obj>)
    target_link_libraries(${PROJECT_NAME}-shared -lrp_sdk -lcryptopp -lm -lpthread)
    target_link_directories(${PROJECT_NAME}-shared PUBLIC ${INSTALL_DIR}/lib)
    target_link_directories(${PROJECT_NAME}-shared PUBLIC ${INSTALL_DIR}/rp_sdk)

    if(IS_INSTALL)
        install(TARGETS ${PROJECT_NAME}-shared
            LIBRARY DESTINATION ${INSTALL_DIR}/lib/web
            ARCHIVE DESTINATION ${INSTALL_DIR}/lib/web)

        install(FILES ${header}
            DESTINATION ${INSTALL_DIR}/include/web)

    endif()
endif()


if(BUILD_STATIC)
    add_library(${PROJECT_NAME}-static STATIC)
    set_property(TARGET ${PROJECT_NAME}-static PROPERTY OUTPUT_NAME ${PROJECT_NAME})
    target_sources(${PROJECT_NAME}-static PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-obj>)
    target_link_libraries(${PROJECT_NAME}-static -lrp_sdk -lcryptopp -lm -lpthread)
    target_link_directories(${PROJECT_NAME}-static PUBLIC ${INSTALL_DIR}/lib)
    target_link_directories(${PROJECT_NAME}-static PUBLIC ${INSTALL_DIR}/rp_sdk)

    if(IS_INSTALL)
        install(TARGETS ${PROJECT_NAME}-static
            LIBRARY DESTINATION ${INSTALL_DIR}/lib/web
            ARCHIVE DESTINATION ${INSTALL_DIR}/lib/web)

        install(FILES ${header}
            DESTINATION ${INSTALL_DIR}/include/web)
    endif()
endif()

unset(INSTALL_DIR CACHE)