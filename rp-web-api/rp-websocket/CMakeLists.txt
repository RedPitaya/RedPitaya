cmake_minimum_required(VERSION 3.14)
project(rp-websocket)

option(BUILD_SHARED "Builds shared library" ON)
option(BUILD_STATIC "Builds static library" ON)

option(IS_INSTALL "Install library" ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 20)


if(NOT DEFINED INSTALL_DIR)
    message(WARNING,"Installation path not set. Installation will be skipped")
    set(IS_INSTALL OFF)
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

message(STATUS "VERSION=${VERSION}")
message(STATUS "REVISION=${REVISION}")
message(STATUS "BUILD_NUMBER=${BUILD_NUMBER}")
message(STATUS "Install path ${INSTALL_DIR}")

message(STATUS "Compiler C path: ${CMAKE_C_COMPILER}")
message(STATUS "Compiler C ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "Compiler C version: ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "Compiler C is part: ${CMAKE_COMPILER_IS_GNUC}")

message(STATUS "Compiler C++ path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler C++ ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler C++version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Compiler C++ is part: ${CMAKE_COMPILER_IS_GNUCXX}")

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREAD ON)
find_package(Boost 1.74.0 REQUIRED COMPONENTS system thread regex)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
endif()

set(CMAKE_INSTALL_RPATH ${r_paths})

list(APPEND src
    ${CMAKE_SOURCE_DIR}/src/rp_websocket.cpp
    ${CMAKE_SOURCE_DIR}/src/asio/asio_service.cpp
    ${CMAKE_SOURCE_DIR}/src/asio/asio_socket_simple.cpp
)

list(APPEND header
    ${CMAKE_SOURCE_DIR}/src/rp_websocket.h
    ${CMAKE_SOURCE_DIR}/src/asio/asio_service.h
    ${CMAKE_SOURCE_DIR}/src/asio/asio_socket_simple.h
    ${CMAKE_SOURCE_DIR}/src/asio/signal.hpp
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

#add_compile_options(-fsanitize=address)
add_compile_options(-DVERSION=${VERSION} -DREVISION=${REVISION})
add_compile_options(-Wno-psabi $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Debug>:-DTRACE_ENABLE> $<$<CONFIG:Release>:-Os> -ffunction-sections -fdata-sections)

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=cortex-a9 -mfpu=neon-fp16")
    add_compile_definitions(ARCH_ARM)
endif()

add_library(${PROJECT_NAME}-obj OBJECT ${src})

if(BUILD_SHARED)

    add_library(${PROJECT_NAME}-shared SHARED)
    set_property(TARGET ${PROJECT_NAME}-shared PROPERTY OUTPUT_NAME ${PROJECT_NAME})
    target_sources(${PROJECT_NAME}-shared PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-obj>)
    target_link_libraries(${PROJECT_NAME}-shared ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${JSONCPP_LIBRARIES})

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
    target_link_libraries(${PROJECT_NAME}-static ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${JSONCPP_LIBRARIES})

    if(IS_INSTALL)
        install(TARGETS ${PROJECT_NAME}-static
            LIBRARY DESTINATION ${INSTALL_DIR}/lib/web
            ARCHIVE DESTINATION ${INSTALL_DIR}/lib/web)

        install(FILES ${header}
            DESTINATION ${INSTALL_DIR}/include/web)
    endif()
endif()

unset(INSTALL_DIR CACHE)