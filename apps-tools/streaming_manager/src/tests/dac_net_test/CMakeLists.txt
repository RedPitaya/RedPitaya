cmake_minimum_required(VERSION 3.14)
project(dac_net_test)

add_executable(dac_net_test_server server.cpp)

target_compile_options(dac_net_test_server
    PRIVATE -std=c++11 -pedantic -Wextra $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os>)

target_compile_definitions(dac_net_test_server
    PRIVATE ASIO_STANDALONE)

target_include_directories(dac_net_test_server
    PRIVATE
        ${CMAKE_SOURCE_DIR}/libs/src
        ${CMAKE_SOURCE_DIR}/libs/src/common
        ${CMAKE_SOURCE_DIR}/libs/asio/include)


target_link_libraries(dac_net_test_server
    PRIVATE  rpsasrv pthread)


add_executable(dac_net_test_client client.cpp)

target_compile_options(dac_net_test_client
        PRIVATE -std=c++11 -pedantic -Wextra $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os>)

target_compile_definitions(dac_net_test_client
        PRIVATE ASIO_STANDALONE)

target_include_directories(dac_net_test_client
        PRIVATE
        ${CMAKE_SOURCE_DIR}/libs/src
        ${CMAKE_SOURCE_DIR}/libs/src/common
        ${CMAKE_SOURCE_DIR}/libs/asio/include)


target_link_libraries(dac_net_test_client
        PRIVATE  rpsasrv pthread)
