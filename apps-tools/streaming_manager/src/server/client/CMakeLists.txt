cmake_minimum_required(VERSION 3.14)

project(rpsa_client)

add_executable(rpsa_client 
                client_src/search.cpp 
                client_src/remote.cpp 
                client_src/streaming.cpp 
                client_src/dac_streaming.cpp 
                client_src/config.cpp 
                client_src/options.cpp 
                client_src/main.cpp
                client_src/test_helper.cpp)

target_compile_options(rpsa_client
    PRIVATE -std=c++11 $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os> -pedantic -static -static-libstdc++ -static-libgcc)


target_compile_definitions(rpsa_client  PRIVATE ASIO_STANDALONE)

target_include_directories(rpsa_client  PRIVATE   ${CMAKE_SOURCE_DIR}/../libs/src/common ${CMAKE_SOURCE_DIR}/../libs/src  ${CMAKE_SOURCE_DIR}/../libs/asio/include)

target_link_libraries(rpsa_client PRIVATE rpsasrv)
if(NOT WIN32 )
    target_link_libraries(rpsa_client PRIVATE pthread)
else()
    target_link_libraries(rpsa_client PRIVATE pthread wsock32 ws2_32)
endif()


project(rpsa_loopback_test)

add_executable(rpsa_loopback_test 
                loopback_src/options.cpp 
                loopback_src/main.cpp)

target_compile_options(rpsa_loopback_test
    PRIVATE -std=c++11 $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os> -pedantic -static -static-libstdc++ -static-libgcc)


target_compile_definitions(rpsa_loopback_test  PRIVATE ASIO_STANDALONE)

target_include_directories(rpsa_loopback_test  PRIVATE   ${CMAKE_SOURCE_DIR}/../libs/src/common ${CMAKE_SOURCE_DIR}/../libs/src  ${CMAKE_SOURCE_DIR}/../libs/asio/include)

target_link_libraries(rpsa_loopback_test PRIVATE rpsasrv)
if(NOT WIN32 )
    target_link_libraries(rpsa_loopback_test PRIVATE pthread)
else()
    target_link_libraries(rpsa_loopback_test PRIVATE pthread wsock32 ws2_32)
endif()