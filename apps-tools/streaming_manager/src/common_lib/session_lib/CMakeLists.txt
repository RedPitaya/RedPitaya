cmake_minimum_required(VERSION ${CMAKEVERS})

project(session_lib)

message(STATUS "${Yellow}Project=${PROJECT_NAME}${ColourReset}")

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
            ${PROJECT_SOURCE_DIR}/stream_session.h
            ${PROJECT_SOURCE_DIR}/trigger_commands.h
        )

list(APPEND src
            ${PROJECT_SOURCE_DIR}/stream_session.cpp
            ${PROJECT_SOURCE_DIR}/trigger_commands.cpp
         )

target_sources(${PROJECT_NAME} PRIVATE ${src})

target_link_libraries(${PROJECT_NAME}
    PUBLIC config_net_lib streaming_lib dac_streaming_lib gpio_streaming_lib uio_lib settings_lib logger_lib
    PRIVATE pthread stdc++)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${headers}
        "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}"
    COMMENT "Copying ${PROJECT_NAME} public headers to ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME}"
    VERBATIM)

## The tests run the session on the dummy hardware of the host build with a config client over TCP.
if(NOT RP_PLATFORM AND NOT CMAKE_CROSSCOMPILING AND BUILD_UNIT_TESTS)
    enable_testing()
    add_subdirectory(tests)

    ## The suite runs as part of the build, so a regression fails the build.
    add_custom_target(rp_session_lib_run_tests ALL
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running session_lib unit tests (ctest) - the build fails if any test fails"
        VERBATIM)
    add_dependencies(rp_session_lib_run_tests session_lib_tests)
    ## The data ports of the streams are fixed: not at the same time as the suites of the servers
    foreach(suite rp_streaming_lib_run_tests rp_dac_streaming_lib_run_tests rp_gpio_streaming_lib_run_tests)
        if(TARGET ${suite})
            add_dependencies(rp_session_lib_run_tests ${suite})
        endif()
    endforeach()
endif()
