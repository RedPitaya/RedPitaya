cmake_minimum_required(VERSION 3.5)

if( NOT WIN32 )
add_subdirectory(server_linux_test)
endif()

if( NOT WIN32 )
add_subdirectory(server_linux_test2)
endif()
