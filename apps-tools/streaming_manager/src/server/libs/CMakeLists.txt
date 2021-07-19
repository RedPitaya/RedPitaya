cmake_minimum_required(VERSION 2.8)
project(rpsasrv)

message("MODEL=${MODEL}")

add_library(${PROJECT_NAME})

target_compile_options(rpsasrv
    PRIVATE -std=c++11 -O3 -Wall -pedantic -Wextra -fpermissive -D${MODEL})

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    target_compile_options(${PROJECT_NAME}
        PRIVATE -mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC)

    target_compile_definitions(${PROJECT_NAME}
        PRIVATE ARCH_ARM)
endif()

target_compile_definitions(${PROJECT_NAME}
    PUBLIC ASIO_STANDALONE)

target_include_directories(${PROJECT_NAME}
    PUBLIC  ${CMAKE_SOURCE_DIR}/libs/asio/include
            ${CMAKE_SOURCE_DIR}/libs/src
            ${CMAKE_SOURCE_DIR}/libs/src/common)

if (NOT WIN32)
target_sources(${PROJECT_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/libs/src/StreamingManager.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/AsioNet.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/FileLogger.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/Oscilloscope.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/StreamingApplication.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/UioParser.cpp
            # Common
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
            ${CMAKE_SOURCE_DIR}/libs/src/FileLogger.cpp
            # Common
            ${CMAKE_SOURCE_DIR}/libs/src/common/Writer.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/DataType.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/File.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/Reader.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/BinaryStream.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/file_async_writer.cpp
            ${CMAKE_SOURCE_DIR}/libs/src/common/wavWriter.cpp)
endif()


target_link_libraries(${PROJECT_NAME}
    PRIVATE pthread gcc stdc++)

