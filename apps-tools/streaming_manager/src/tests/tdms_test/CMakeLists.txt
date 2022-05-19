cmake_minimum_required(VERSION 3.14)
project(tdms_test)

message(${CMAKE_BINARY_DIR})

add_executable(tdms_test main.cpp)

target_compile_options(tdms_test
    PRIVATE -std=c++11 -pedantic -Wextra $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os>)

target_compile_definitions(tdms_test
    PRIVATE ASIO_STANDALONE)

target_include_directories(tdms_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/libs/src
        ${CMAKE_SOURCE_DIR}/libs/src/common
        ${CMAKE_SOURCE_DIR}/libs/src/common/TDMS
        ${CMAKE_SOURCE_DIR}/libs/asio/include)

target_link_libraries(tdms_test
    PRIVATE  rpsasrv pthread)
