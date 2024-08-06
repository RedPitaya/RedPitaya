cmake_minimum_required(VERSION 3.18)

project(converter_lib)



message(STATUS "Project=${PROJECT_NAME}")
message(STATUS "RedPitaya platform=${RP_PLATFORM}")
message(STATUS "VERSION=${VERSION}")
message(STATUS "REVISION=${REVISION}")

message(STATUS "Compiler С path: ${CMAKE_C_COMPILER}")
message(STATUS "Compiler С ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "Compiler С version: ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "Compiler С is part: ${CMAKE_COMPILER_IS_GNUC}")

message(STATUS "Compiler С++ path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler С++ ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler С++version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Compiler С++ is part: ${CMAKE_COMPILER_IS_GNUCXX}")


set(CMAKE_CXX_STANDARD 17)

FILE(GLOB_RECURSE INC_ALL "*.h")

add_library(${PROJECT_NAME} ${INC_ALL})

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    target_compile_options(${PROJECT_NAME}
        PRIVATE -mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC)

    target_compile_definitions(${PROJECT_NAME}
        PRIVATE ARCH_ARM)
endif()

if (RP_PLATFORM)
    target_compile_options(${PROJECT_NAME} PRIVATE -DRP_PLATFORM)
endif()

if (RP_PLATFORM)
    target_compile_options(${PROJECT_NAME} PRIVATE -DRP_PLATFORM)
endif()


target_compile_options(${PROJECT_NAME}
    PRIVATE -std=c++17 -Wall -pedantic -Wextra -fpermissive $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Debug>:-DTRACE_ENABLE> $<$<CONFIG:Release>:-Os> -ffunction-sections -fdata-sections)


target_include_directories(${PROJECT_NAME}
    PUBLIC  ${PROJECT_SOURCE_DIR}
            ${COMMON_LIB_DIR}
            ${CMAKE_SOURCE_DIR}
            )

target_compile_definitions(${PROJECT_NAME}
    PUBLIC ASIO_STANDALONE)

list(APPEND headers
            ${PROJECT_SOURCE_DIR}/converter.h
        )

list(APPEND src
            ${PROJECT_SOURCE_DIR}/converter.cpp
         )

target_sources(${PROJECT_NAME} PRIVATE ${src})

target_link_libraries(${PROJECT_NAME}
    PUBLIC data_lib
    PRIVATE pthread stdc++)


file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${headers}
        "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}"
    COMMENT "Copying ${PROJECT_NAME} public headers to ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME}"
    VERBATIM)
