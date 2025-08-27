cmake_minimum_required(VERSION ${CMAKEVERS})

project(net_lib)

message(STATUS "${Yellow}Project=${PROJECT_NAME}${ColourReset}")
message(STATUS "RedPitaya platform=${RP_PLATFORM}")
message(STATUS "CPU ARCH=${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "VERSION=${VERSION}")
message(STATUS "REVISION=${REVISION}")
message(STATUS "BUILD_NUMBER=${BUILD_NUMBER}")
message(STATUS "LINUX_VER=${LINUX_VER}")

message(STATUS "Compiler C path: ${CMAKE_C_COMPILER}")
message(STATUS "Compiler C ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "Compiler C version: ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "Compiler C is part: ${CMAKE_COMPILER_IS_GNUC}")

message(STATUS "Compiler C++ path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler C++ ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler C++version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Compiler C++ is part: ${CMAKE_COMPILER_IS_GNUCXX}")

FILE(GLOB_RECURSE INC_ALL "*.h")

add_library(${PROJECT_NAME} ${INC_ALL})

target_include_directories(${PROJECT_NAME}
    PUBLIC  ${PROJECT_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}
            ${COMMON_LIB_DIR}
            ${CMAKE_BINARY_DIR}/asio/include
            )

target_compile_definitions(${PROJECT_NAME}
    PUBLIC ASIO_STANDALONE)

list(APPEND headers
            ${PROJECT_SOURCE_DIR}/asio_service.h
            ${PROJECT_SOURCE_DIR}/asio_net.h
            ${PROJECT_SOURCE_DIR}/asio_net_simple.h
            ${PROJECT_SOURCE_DIR}/asio_socket_dma.h
            ${PROJECT_SOURCE_DIR}/asio_socket_simple.h
            ${PROJECT_SOURCE_DIR}/asio_common.h
        )

list(APPEND src
            ${PROJECT_SOURCE_DIR}/asio_service.cpp
            ${PROJECT_SOURCE_DIR}/asio_net.cpp
            ${PROJECT_SOURCE_DIR}/asio_net_simple.cpp
            ${PROJECT_SOURCE_DIR}/asio_socket_dma.cpp
            ${PROJECT_SOURCE_DIR}/asio_socket_simple.cpp
            ${PROJECT_SOURCE_DIR}/asio_common.cpp
        )

target_sources(${PROJECT_NAME} PRIVATE ${src})

target_link_libraries(${PROJECT_NAME} PUBLIC data_lib)

if(NOT WIN32 )
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread stdc++)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread stdc++ wsock32 ws2_32)
endif()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${headers}
        "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}"
    COMMENT "Copying ${PROJECT_NAME} public headers to ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME}"
    VERBATIM)
