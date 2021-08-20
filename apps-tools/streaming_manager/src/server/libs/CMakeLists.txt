cmake_minimum_required(VERSION 3.14)
project(rpsasrv)

message("MODEL=${MODEL}")

set(CMAKE_CXX_STANDARD 11)



include(FetchContent)

set (BUILD_SHARED_LIBS OFF CACHE INTERNAL "Turn off shared")
set (JSONCPP_WITH_TESTS OFF CACHE INTERNAL "Turn off jsontest")

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC")
endif()


FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp
        GIT_TAG 1.9.4
        )

FetchContent_MakeAvailable(json)

#FetchContent_GetProperties(json)
#if(NOT json_POPULATED)
#    FetchContent_Populate(json)
#    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
#endif()

add_library(${PROJECT_NAME})

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    target_compile_options(${PROJECT_NAME}
        PRIVATE -mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC)

    target_compile_definitions(${PROJECT_NAME}
        PRIVATE ARCH_ARM)
endif()

target_compile_options(rpsasrv
    PRIVATE -std=c++11 -Wall -pedantic -Wextra -fpermissive -D${MODEL} $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os>)



target_compile_definitions(${PROJECT_NAME}
    PUBLIC ASIO_STANDALONE)

target_include_directories(${PROJECT_NAME}
    PUBLIC  ${CMAKE_SOURCE_DIR}/libs/asio/include
            ${CMAKE_SOURCE_DIR}/libs/src
            ${CMAKE_SOURCE_DIR}/libs/src/common
            ${json_SOURCE_DIR}/include)

if (NOT WIN32)
target_sources(${PROJECT_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/libs/src/StreamingManager.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/AsioNet.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/AsioSocket.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/AsioNetSimple.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/AsioBroadcastSocket.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/AsioSocketSimple.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/FileLogger.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/Oscilloscope.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/StreamingApplication.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/UioParser.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/NetConfigManager.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/ServerNetConfigManager.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/ClientNetConfigManager.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/DACAsioNetController.cpp
            # Common
            ${CMAKE_SOURCE_DIR}/libs/src/common/stream_settings.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/Writer.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/DataType.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/File.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/Reader.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/BinaryStream.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/file_async_writer.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/wavWriter.cpp)
else()
target_sources(${PROJECT_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/libs/src/StreamingManager.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/AsioNet.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/AsioSocket.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/AsioBroadcastSocket.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/AsioNetSimple.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/AsioSocketSimple.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/FileLogger.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/NetConfigManager.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/ServerNetConfigManager.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/ClientNetConfigManager.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/DACAsioNetController.cpp
            # Common
            ${CMAKE_SOURCE_DIR}/libs/src/common/stream_settings.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/Writer.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/DataType.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/File.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/Reader.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/BinaryStream.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/file_async_writer.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/wavWriter.cpp)
endif()

add_library(jsoncpp STATIC IMPORTED)
set_property(TARGET jsoncpp PROPERTY
        IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/lib/libjsoncpp.a)

target_link_libraries(${PROJECT_NAME}
    PRIVATE pthread gcc stdc++ ${Boost_LIBRARIES} jsoncpp)
