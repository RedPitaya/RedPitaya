cmake_minimum_required(VERSION 3.14)
project(server_linux_test)

add_executable(server_linux_test server.cpp)

target_compile_options(server_linux_test
    PRIVATE -std=c++11 -pedantic -Wextra $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os>)

target_compile_definitions(server_linux_test
    PRIVATE ASIO_STANDALONE)

target_include_directories(server_linux_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/libs/src
        ${CMAKE_SOURCE_DIR}/libs/src/common
        ${CMAKE_SOURCE_DIR}/libs/asio/include)


target_link_libraries(server_linux_test
    PRIVATE  rpsasrv pthread)
