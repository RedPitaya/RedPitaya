cmake_minimum_required(VERSION ${CMAKEVERS})

project(gpio_streaming_lib)

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
            ${COMMON_LIB_DIR}
            ${CMAKE_SOURCE_DIR}
            )

target_compile_definitions(${PROJECT_NAME}
    PUBLIC ASIO_STANDALONE)

list(APPEND headers
            ${PROJECT_SOURCE_DIR}/gpio_stop_reason.h
            ${PROJECT_SOURCE_DIR}/gpio_in_stream_server.h
            ${PROJECT_SOURCE_DIR}/gpio_out_stream_server.h
        )

list(APPEND src
            ${PROJECT_SOURCE_DIR}/gpio_in_stream_server.cpp
            ${PROJECT_SOURCE_DIR}/gpio_out_stream_server.cpp
         )

target_sources(${PROJECT_NAME} PRIVATE ${src})

target_link_libraries(${PROJECT_NAME}
    PUBLIC data_lib net_lib uio_lib settings_lib logger_lib writer_lib reader_lib bin_lib
    PRIVATE pthread stdc++)


file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${headers}
        "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}"
    COMMENT "Copying ${PROJECT_NAME} public headers to ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME}"
    VERBATIM)

## The tests run the server on the dummy CGpio of the host build, with clients over TCP.
if(NOT RP_PLATFORM AND NOT CMAKE_CROSSCOMPILING AND BUILD_UNIT_TESTS)
    enable_testing()
    add_subdirectory(tests)

    ## The suite runs as part of the build, so a regression fails the build.
    add_custom_target(rp_gpio_streaming_lib_run_tests ALL
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running gpio_streaming_lib unit tests (ctest) - the build fails if any test fails"
        VERBATIM)
    add_dependencies(rp_gpio_streaming_lib_run_tests gpio_streaming_lib_tests)
endif()
