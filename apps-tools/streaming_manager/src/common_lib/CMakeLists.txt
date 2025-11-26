cmake_minimum_required(VERSION 3.18)
project(common_lib)

set(CMAKE_CXX_STANDARD 20)
set(COMMON_LIB_DIR ${PROJECT_SOURCE_DIR})

add_subdirectory(tdms_lib)
add_subdirectory(settings_lib)
add_subdirectory(wav_lib)
add_subdirectory(data_lib)
add_subdirectory(reader_lib)
add_subdirectory(writer_lib)
add_subdirectory(net_lib)
add_subdirectory(broadcast_lib)
add_subdirectory(logger_lib)
add_subdirectory(uio_lib)
add_subdirectory(config_net_lib)
add_subdirectory(streaming_lib)
add_subdirectory(dac_streaming_lib)
add_subdirectory(converter_lib)

add_custom_target( asio ALL)
add_custom_command(TARGET asio PRE_BUILD
   COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/asio/
   COMMAND ${CMAKE_COMMAND} -E tar xzf ${PROJECT_SOURCE_DIR}/asio.zip
WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
DEPENDS ${PROJECT_SOURCE_DIR}/asio.zip
COMMENT "Unpacking asio.zip"
VERBATIM)

add_dependencies(broadcast_lib asio data_lib)
add_dependencies(config_net_lib asio data_lib)
add_dependencies(net_lib asio)
add_dependencies(streaming_lib net_lib)
add_dependencies(data_lib uio_lib logger_lib)

add_custom_target(${PROJECT_NAME})
