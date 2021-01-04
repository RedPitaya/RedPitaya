cmake_minimum_required(VERSION 2.8)
project(rpsa_client)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

add_executable(rpsa_client client.cpp)

target_compile_options(rpsa_client
    PRIVATE -std=c++11 -O3 -pedantic -static -static-libstdc++ -static-libgcc)


target_compile_definitions(rpsa_client  PRIVATE ASIO_STANDALONE)

target_include_directories(rpsa_client  PRIVATE  ${CMAKE_SOURCE_DIR}/../include  ${CMAKE_SOURCE_DIR}/../include/rpsa/common/core ${CMAKE_SOURCE_DIR}/../include/rpsa/server/core  ${CMAKE_SOURCE_DIR}/../libs/asio/include)

add_library(rpsasrv STATIC IMPORTED)
set_property(TARGET rpsasrv PROPERTY
             IMPORTED_LOCATION ../bin/librpsasrv.a)

target_link_libraries(rpsa_client PRIVATE rpsasrv)
if(NOT WIN32 )
    target_link_libraries(rpsa_client PRIVATE pthread)
else()
    target_link_libraries(rpsa_client PRIVATE pthread wsock32 ws2_32)
endif()


