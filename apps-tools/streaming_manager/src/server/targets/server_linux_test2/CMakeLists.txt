cmake_minimum_required(VERSION 3.5)
project(server_linux_test2)

add_executable(server_linux_test2 server.cpp)

target_compile_options(server_linux_test2
    PRIVATE -std=c++11 -pedantic -Wextra)

target_compile_definitions(server_linux_test2
    PRIVATE ASIO_STANDALONE)

target_include_directories(server_linux_test2
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/libs/asio/include)


target_link_libraries(server_linux_test2
    PRIVATE  rpsasrv pthread)
