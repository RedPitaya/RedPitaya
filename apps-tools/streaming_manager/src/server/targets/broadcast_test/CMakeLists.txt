cmake_minimum_required(VERSION 3.14)
project(broadcast_test)

add_executable(broadcast_server_test server.cpp)

target_compile_options(broadcast_server_test
    PRIVATE -std=c++11 -pedantic -Wextra $<$<CONFIG:Debug>:-fsanitize=address -g3> $<$<CONFIG:Release>:-Os>)

target_compile_definitions(broadcast_server_test
    PRIVATE ASIO_STANDALONE)

target_include_directories(broadcast_server_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/libs/src
        ${CMAKE_SOURCE_DIR}/libs/src/common
        ${CMAKE_SOURCE_DIR}/libs/asio/include)


target_link_libraries(broadcast_server_test
    PRIVATE  rpsasrv pthread)


add_executable(broadcast_client_test client.cpp)

target_compile_options(broadcast_client_test
        PRIVATE -std=c++11 -pedantic -Wextra $<$<CONFIG:Debug>:-fsanitize=address -g3> $<$<CONFIG:Release>:-Os>)

target_compile_definitions(broadcast_client_test
        PRIVATE ASIO_STANDALONE)

target_include_directories(broadcast_client_test
        PRIVATE
        ${CMAKE_SOURCE_DIR}/libs/src
        ${CMAKE_SOURCE_DIR}/libs/src/common
        ${CMAKE_SOURCE_DIR}/libs/asio/include)


target_link_libraries(broadcast_client_test
        PRIVATE  rpsasrv pthread)