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
            ${CMAKE_SOURCE_DIR}/common
            ${CMAKE_SOURCE_DIR}/include
	        ${CMAKE_SOURCE_DIR}/include/rpsa/server/core
	        ${CMAKE_SOURCE_DIR}/include/rpsa/common/core
            ${CMAKE_SOURCE_DIR}/include/rpsa/common/xml)

if (NOT WIN32)
target_sources(${PROJECT_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/src/rpsa/server/core/StreamingManager.cpp
            ${CMAKE_SOURCE_DIR}/src/rpsa/server/core/AsioNet.cpp
            ${CMAKE_SOURCE_DIR}/src/rpsa/server/core/FileLogger.cpp
            # Common
            ${CMAKE_SOURCE_DIR}/src/rpsa/common/core/Writer.cpp
            ${CMAKE_SOURCE_DIR}/src/rpsa/common/core/DataType.cpp
            ${CMAKE_SOURCE_DIR}/src/rpsa/common/core/File.cpp
            ${CMAKE_SOURCE_DIR}/src/rpsa/common/core/Reader.cpp
            ${CMAKE_SOURCE_DIR}/src/rpsa/common/core/BinaryStream.cpp
            ${CMAKE_SOURCE_DIR}/src/rpsa/common/core/file_async_writer.cpp
            ${CMAKE_SOURCE_DIR}/src/rpsa/common/core/wavWriter.cpp
            ${CMAKE_SOURCE_DIR}/src/rpsa/server/core/Oscilloscope.cpp
            ${CMAKE_SOURCE_DIR}/src/rpsa/server/core/StreamingApplication.cpp
            ${CMAKE_SOURCE_DIR}/src/rpsa/server/core/UioParser.cpp)
else()
target_sources(${PROJECT_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/src/rpsa/server/core/StreamingManager.cpp
            ${CMAKE_SOURCE_DIR}/src/rpsa/server/core/AsioNet.cpp
            ${CMAKE_SOURCE_DIR}/src/rpsa/server/core/FileLogger.cpp
            # Common
            ${CMAKE_SOURCE_DIR}/src/rpsa/common/core/Writer.cpp
            ${CMAKE_SOURCE_DIR}/src/rpsa/common/core/DataType.cpp
            ${CMAKE_SOURCE_DIR}/src/rpsa/common/core/File.cpp
            ${CMAKE_SOURCE_DIR}/src/rpsa/common/core/Reader.cpp
            ${CMAKE_SOURCE_DIR}/src/rpsa/common/core/BinaryStream.cpp
            ${CMAKE_SOURCE_DIR}/src/rpsa/common/core/file_async_writer.cpp
            ${CMAKE_SOURCE_DIR}/src/rpsa/common/core/wavWriter.cpp)
endif()


target_link_libraries(${PROJECT_NAME}
    PRIVATE pthread gcc stdc++)

