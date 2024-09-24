cmake_minimum_required(VERSION ${CMAKEVERS})

project(uio_lib)

message(STATUS "${Yellow}Project=${PROJECT_NAME}${ColourReset}")
message(STATUS "RedPitaya platform=${RP_PLATFORM}")
message(STATUS "CPU ARCH=${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "VERSION=${VERSION}")
message(STATUS "REVISION=${REVISION}")
message(STATUS "BUILD_NUMBER=${BUILD_NUMBER}")

message(STATUS "Compiler С path: ${CMAKE_C_COMPILER}")
message(STATUS "Compiler С ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "Compiler С version: ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "Compiler С is part: ${CMAKE_COMPILER_IS_GNUC}")

message(STATUS "Compiler С++ path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler С++ ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler С++version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Compiler С++ is part: ${CMAKE_COMPILER_IS_GNUCXX}")

FILE(GLOB_RECURSE INC_ALL "*.h")

add_library(${PROJECT_NAME} ${INC_ALL})

target_compile_options(${PROJECT_NAME}
    PRIVATE -Wno-unused-result)

target_include_directories(${PROJECT_NAME}
    PUBLIC  ${PROJECT_SOURCE_DIR}
            ${COMMON_LIB_DIR}
            ${CMAKE_SOURCE_DIR}
      )

list(APPEND headers
            ${PROJECT_SOURCE_DIR}/generator.h
            ${PROJECT_SOURCE_DIR}/oscilloscope.h
            ${PROJECT_SOURCE_DIR}/memory_manager.h
            ${PROJECT_SOURCE_DIR}/uio_parser.h
      )

if(RP_PLATFORM)
    list(APPEND src
            ${PROJECT_SOURCE_DIR}/generator.cpp
            ${PROJECT_SOURCE_DIR}/oscilloscope.cpp
            ${PROJECT_SOURCE_DIR}/memory_manager_static.cpp
            ${PROJECT_SOURCE_DIR}/uio_parser.cpp
        )
else()
    list(APPEND src
            ${PROJECT_SOURCE_DIR}/generator_dummy.cpp
            ${PROJECT_SOURCE_DIR}/oscilloscope_dummy.cpp
            ${PROJECT_SOURCE_DIR}/memory_manager_dynamic.cpp
            ${PROJECT_SOURCE_DIR}/uio_parser.cpp
        )
endif()

target_sources(${PROJECT_NAME} PRIVATE ${src})

target_link_libraries(${PROJECT_NAME}
    PRIVATE pthread stdc++)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${headers}
        "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}"
    COMMENT "Copying ${PROJECT_NAME} public headers to ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME}"
    VERBATIM)
