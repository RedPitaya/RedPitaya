cmake_minimum_required(VERSION ${CMAKEVERS})

project(web_lib)

## The link of the streaming server to its web page: a WebSocket server on the loopback address,
## nginx proxies the page to it.

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
            ${PROJECT_SOURCE_DIR}/websocket_server.h
        )

list(APPEND src
            ${PROJECT_SOURCE_DIR}/websocket_server.cpp
        )

target_sources(${PROJECT_NAME} PRIVATE ${src})

target_link_libraries(${PROJECT_NAME}
    PUBLIC logger_lib
    PRIVATE pthread stdc++)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${headers}
        "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}"
    COMMENT "Copying ${PROJECT_NAME} public headers to ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME}"
    VERBATIM)

if(NOT CMAKE_CROSSCOMPILING AND BUILD_UNIT_TESTS)
    enable_testing()
    add_subdirectory(tests)

    ## The suite runs as part of the build, so a regression fails the build.
    add_custom_target(rp_web_lib_run_tests ALL
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running web_lib unit tests (ctest) - the build fails if any test fails"
        VERBATIM)
    add_dependencies(rp_web_lib_run_tests web_lib_tests)
endif()
