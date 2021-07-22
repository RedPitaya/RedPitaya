cmake_minimum_required(VERSION 3.14)
project(settings_test)

add_executable(settings_test server.cpp)

target_compile_options(settings_test
    PRIVATE -std=c++11 -pedantic -Wextra)

target_compile_definitions(settings_test
    PRIVATE ASIO_STANDALONE)

target_include_directories(settings_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/libs/src
        ${CMAKE_SOURCE_DIR}/libs/src/common
        ${CMAKE_SOURCE_DIR}/libs/asio/include)


target_link_libraries(settings_test
    PRIVATE  rpsasrv pthread)
