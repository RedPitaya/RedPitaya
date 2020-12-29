cmake_minimum_required(VERSION 2.8)
project(convert_tool)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

add_executable(convert_tool main.cpp)

target_compile_options(convert_tool
    PRIVATE -std=c++11 -pedantic -static -O3 -static-libstdc++ -static-libgcc)


target_compile_definitions(convert_tool  PRIVATE ASIO_STANDALONE)

target_include_directories(convert_tool  PRIVATE  ${CMAKE_SOURCE_DIR}/../include  ${CMAKE_SOURCE_DIR}/../include/rpsa/common/core ${CMAKE_SOURCE_DIR}/../include/rpsa/server/core  ${CMAKE_SOURCE_DIR}/../libs/asio/include)

add_library(rpsasrv STATIC IMPORTED)
set_property(TARGET rpsasrv PROPERTY
             IMPORTED_LOCATION ../bin/librpsasrv.a)

target_link_libraries(convert_tool PRIVATE rpsasrv)
if(NOT WIN32 )
    target_link_libraries(convert_tool PRIVATE pthread)
else()
    target_link_libraries(convert_tool PRIVATE pthread wsock32 ws2_32)
endif()


