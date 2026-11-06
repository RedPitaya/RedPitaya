cmake_minimum_required(VERSION ${CMAKEVERS})

project(bin_lib)

message(STATUS "${Yellow}Project=${PROJECT_NAME}${ColourReset}")

FILE(GLOB_RECURSE INC_ALL "*.h")

add_library(${PROJECT_NAME} ${INC_ALL})

target_include_directories(${PROJECT_NAME}
    PUBLIC  ${PROJECT_SOURCE_DIR}
            ${COMMON_LIB_DIR}
            ${CMAKE_SOURCE_DIR}
    )

list(APPEND headers
            ${PROJECT_SOURCE_DIR}/bin_format.h
            ${PROJECT_SOURCE_DIR}/bin_file.h
            ${PROJECT_SOURCE_DIR}/gpio_words.h
        )

list(APPEND src
            ${PROJECT_SOURCE_DIR}/bin_file.cpp
            ${PROJECT_SOURCE_DIR}/gpio_words.cpp
        )

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

## The tests write files and read them back on the host.
if(NOT RP_PLATFORM AND NOT CMAKE_CROSSCOMPILING AND BUILD_UNIT_TESTS)
    enable_testing()
    add_subdirectory(tests)

    ## The suite runs as part of the build, so a regression fails the build.
    add_custom_target(rp_bin_lib_run_tests ALL
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running bin_lib unit tests (ctest) - the build fails if any test fails"
        VERBATIM)
    add_dependencies(rp_bin_lib_run_tests bin_lib_tests)
endif()
