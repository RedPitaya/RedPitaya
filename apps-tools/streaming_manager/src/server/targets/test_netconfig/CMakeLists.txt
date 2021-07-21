cmake_minimum_required(VERSION 3.20)
project(net_configTest)

add_executable(net_configTest server.cpp)

target_compile_options(net_configTest
    PRIVATE -std=c++11 -pedantic -Wextra)

target_compile_definitions(net_configTest
    PRIVATE ASIO_STANDALONE)

target_include_directories(net_configTest
    PRIVATE
        ${CMAKE_SOURCE_DIR}/libs/src
        ${CMAKE_SOURCE_DIR}/libs/src/common
        ${CMAKE_SOURCE_DIR}/libs/asio/include)


target_link_libraries(net_configTest
    PRIVATE  rpsasrv pthread)
