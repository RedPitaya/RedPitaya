cmake_minimum_required(VERSION 2.8)
project(rpsa_client)

add_executable(rpsa_client client.cpp)

target_compile_options(rpsa_client
    PRIVATE -std=c++11 -pedantic -static -static-libstdc++ -static-libgcc)


target_compile_definitions(rpsa_client  PRIVATE ASIO_STANDALONE)

target_include_directories(rpsa_client  PRIVATE  ${CMAKE_SOURCE_DIR}/include  ${CMAKE_SOURCE_DIR}/libs/asio/include)

target_link_libraries(rpsa_client PRIVATE rpsasrv)
if(NOT WIN32 )
    target_link_libraries(rpsa_client PRIVATE pthread)
else()
    target_link_libraries(rpsa_client PRIVATE pthread wsock32 ws2_32)
endif()


