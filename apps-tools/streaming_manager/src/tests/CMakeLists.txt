cmake_minimum_required(VERSION 3.14)
project(server_linux_test_dir)

if( NOT WIN32 )
add_subdirectory(server_linux_test)
endif()

if( NOT WIN32 )
add_subdirectory(server_linux_test2)
endif()

if( NOT WIN32 )
add_subdirectory(server_linux_master_slave_test)
endif()

if( NOT WIN32 )
add_subdirectory(server_dac_test)
endif()

if( NOT WIN32 )
    add_subdirectory(dac_net_test)
endif()

if( NOT WIN32 )
add_subdirectory(broadcast_test)
endif()

if( NOT WIN32 )
add_subdirectory(settings_test)
endif()

if( NOT WIN32 )
    add_subdirectory(tdms_test)
endif()

if( NOT WIN32 )
    add_subdirectory(reader_controller_test)
endif()


