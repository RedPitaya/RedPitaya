cmake_minimum_required(VERSION 3.14)
project(streaming_root)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_CXX_STANDARD 11)

add_subdirectory(libs)
add_subdirectory(targets)

if (CLIENT_PRJ)
add_subdirectory(client)
add_subdirectory(convert_tool)
endif()