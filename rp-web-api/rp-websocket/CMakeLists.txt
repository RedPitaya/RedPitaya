cmake_minimum_required(VERSION 3.14)
project(rp-websocket)

option(BUILD_SHARED "Builds shared library" ON)
option(BUILD_STATIC "Builds static library" ON)

option(IS_INSTALL "Install library" ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

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
message(STATUS "LINUX_VER=${LINUX_VER}")
message(STATUS "Install path ${INSTALL_DIR}")

message(STATUS "Compiler C path: ${CMAKE_C_COMPILER}")
message(STATUS "Compiler C ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "Compiler C version: ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "Compiler C is part: ${CMAKE_COMPILER_IS_GNUC}")

message(STATUS "Compiler C++ path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler C++ ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler C++version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Compiler C++ is part: ${CMAKE_COMPILER_IS_GNUCXX}")

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message(STATUS "Debug mode")
endif()

include(FetchContent)

FetchContent_Declare(websocketpp
    GIT_REPOSITORY https://github.com/RedPitaya/websocketpp.git
    EXCLUDE_FROM_ALL
)

if(NOT websocketpp_POPULATED)
    FetchContent_Populate(websocketpp)
    add_subdirectory(${websocketpp_SOURCE_DIR} ${websocketpp_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()


set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-psabi")


FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp
    GIT_TAG 1.9.4
    EXCLUDE_FROM_ALL
)

if(NOT json_POPULATED)
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

include_directories(${websocketpp_SOURCE_DIR})
include_directories(${json_SOURCE_DIR}/include)

list(APPEND r_paths
    /opt/redpitaya/lib
    /opt/redpitaya/lib/web
)

set(CMAKE_INSTALL_RPATH ${r_paths})

list(APPEND src
    ${CMAKE_SOURCE_DIR}/src/rp_websocket.cpp
    ${CMAKE_SOURCE_DIR}/src/websocket_server.cpp
    ${json_SOURCE_DIR}/src/lib_json/json_reader.cpp
    ${json_SOURCE_DIR}/src/lib_json/json_writer.cpp
    ${json_SOURCE_DIR}/src/lib_json/json_value.cpp
)

list(APPEND header
    ${CMAKE_SOURCE_DIR}/src/rp_websocket.h
    ${CMAKE_SOURCE_DIR}/src/signal.hpp
)


#add_compile_options(-fsanitize=address)
add_compile_options(-DVERSION=${VERSION} -DREVISION=${REVISION})
add_compile_options(-mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC)
add_compile_options($<$<CONFIG:Debug>:-g3>)
add_compile_options($<$<CONFIG:Debug>:-DTRACE_ENABLE> $<$<CONFIG:Release>:-Os> -ffunction-sections -fdata-sections)
add_compile_definitions(ARCH_ARM)

add_library(${PROJECT_NAME}-obj OBJECT ${src})

# Need for optimize RAM
set_property(TARGET ${PROJECT_NAME}-obj PROPERTY EXCLUDE_FROM_DEFAULT_BUILD_DEBUG TRUE)

target_include_directories(${PROJECT_NAME}-obj PUBLIC ${INSTALL_DIR}/include)

if(BUILD_SHARED)
    add_library(${PROJECT_NAME}-shared SHARED)
    set_property(TARGET ${PROJECT_NAME}-shared PROPERTY OUTPUT_NAME ${PROJECT_NAME})
    target_sources(${PROJECT_NAME}-shared PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-obj>)
    # target_link_libraries(${PROJECT_NAME}-shared)

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
    # target_link_libraries(${PROJECT_NAME}-static)

    if(IS_INSTALL)
        install(TARGETS ${PROJECT_NAME}-static
            LIBRARY DESTINATION ${INSTALL_DIR}/lib/web
            ARCHIVE DESTINATION ${INSTALL_DIR}/lib/web)

        install(FILES ${header}
            DESTINATION ${INSTALL_DIR}/include/web)
    endif()
endif()

unset(INSTALL_DIR CACHE)

