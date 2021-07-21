cmake_minimum_required(VERSION 3.20)
project(server_linux_test_dir)

if( NOT WIN32 )
add_subdirectory(server_linux_test)
endif()

if( NOT WIN32 )
add_subdirectory(server_linux_test2)
endif()

if( NOT WIN32 )
add_subdirectory(test_netconfig)
endif()
