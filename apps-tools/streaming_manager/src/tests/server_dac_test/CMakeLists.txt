cmake_minimum_required(VERSION 3.14)
project(server_dac_test)

add_executable(server_dac_test server.cpp)

target_compile_options(server_dac_test
    PRIVATE -std=c++11 -pedantic -Wextra $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os>)

target_compile_definitions(server_dac_test
    PRIVATE ASIO_STANDALONE)

target_include_directories(server_dac_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/libs/src
        ${CMAKE_SOURCE_DIR}/libs/src/common
        ${CMAKE_SOURCE_DIR}/libs/asio/include)


target_link_libraries(server_dac_test
    PRIVATE  rpsasrv pthread)

add_custom_command(
    TARGET server_dac_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            test.wav
            ${CMAKE_BINARY_DIR}/bin/test.wav)