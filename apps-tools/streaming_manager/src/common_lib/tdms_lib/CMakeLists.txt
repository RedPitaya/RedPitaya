## CMAKEVERS is supplied by the parent RedPitaya build. Default it only when it
## is absent, so this directory can also be configured on its own (that is what
## the unit tests are built from) while an in-tree build keeps honouring the
## parent's value verbatim - replacing this with a hardcoded version would reset
## this directory's policy defaults and could change the parent's behaviour.
if(NOT DEFINED CMAKEVERS)
    set(CMAKEVERS 3.16)
endif()
cmake_minimum_required(VERSION ${CMAKEVERS})

project(tdms_lib)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(TDMS_STANDALONE TRUE)
else()
    set(TDMS_STANDALONE FALSE)
endif()

option(BUILD_UNIT_TESTS "Build tdms_lib unit tests (GoogleTest)" ON)
option(TDMS_ENABLE_SANITIZERS "Build tdms_lib and its tests with ASan+UBSan" OFF)

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

message(STATUS "Build ${PROJECT_NAME} unit tests (GoogleTest): ${BUILD_UNIT_TESTS}")
message(STATUS "Build ${PROJECT_NAME} and its tests with ASan+UBSan: ${TDMS_ENABLE_SANITIZERS}")

message(STATUS "Host system name=${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "Host system processor=${CMAKE_HOST_SYSTEM_PROCESSOR}")

message(STATUS "Target system name=${CMAKE_SYSTEM_NAME}")
message(STATUS "Target system processor=${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Cross compiling=${CMAKE_CROSSCOMPILING}")


FILE(GLOB_RECURSE INC_ALL "*.h")

add_library(${PROJECT_NAME} ${INC_ALL})

## COMMON_LIB_DIR and CMAKE_SOURCE_DIR come from the parent tree and expand to
## nothing standalone; an empty entry here is an error on some CMake versions.
## No source in this library includes anything from either, so filtering them is
## behaviour-preserving in-tree.
set(_tdms_include_dirs ${PROJECT_SOURCE_DIR})
foreach(_dir ${COMMON_LIB_DIR} ${CMAKE_SOURCE_DIR})
    if(_dir AND IS_ABSOLUTE "${_dir}")
        list(APPEND _tdms_include_dirs "${_dir}")
    endif()
endforeach()
if(TDMS_STANDALONE)
    ## So the tests can use the consumer's own include form, "tdms_lib/file.h".
    get_filename_component(_tdms_parent_dir "${PROJECT_SOURCE_DIR}" DIRECTORY)
    list(APPEND _tdms_include_dirs "${_tdms_parent_dir}")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${_tdms_include_dirs})

## Floor, not a pin: std::shared_ptr<uint8_t[]> (writer.h, data_type.h) is C++17.
## A no-op when the parent already requires a newer standard.
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

list(APPEND headers
            ${PROJECT_SOURCE_DIR}/writer.h
            ${PROJECT_SOURCE_DIR}/data_type.h
            ${PROJECT_SOURCE_DIR}/file.h
            ${PROJECT_SOURCE_DIR}/reader.h
            ${PROJECT_SOURCE_DIR}/binary_stream.h
            ${PROJECT_SOURCE_DIR}/file_struct_types.h
            ${PROJECT_SOURCE_DIR}/tdms.h
            ${PROJECT_SOURCE_DIR}/value.h
            ${PROJECT_SOURCE_DIR}/segment_builder.h
            ${PROJECT_SOURCE_DIR}/file_writer.h
            ${PROJECT_SOURCE_DIR}/file_reader.h
            ${PROJECT_SOURCE_DIR}/waveform.h
        )

list(APPEND src
            ${PROJECT_SOURCE_DIR}/writer.cpp
            ${PROJECT_SOURCE_DIR}/data_type.cpp
            ${PROJECT_SOURCE_DIR}/file.cpp
            ${PROJECT_SOURCE_DIR}/reader.cpp
            ${PROJECT_SOURCE_DIR}/binary_stream.cpp
            ${PROJECT_SOURCE_DIR}/value.cpp
            ${PROJECT_SOURCE_DIR}/path.cpp
            ${PROJECT_SOURCE_DIR}/segment_builder.cpp
            ${PROJECT_SOURCE_DIR}/file_writer.cpp
            ${PROJECT_SOURCE_DIR}/file_reader.cpp
        )

target_sources(${PROJECT_NAME} PRIVATE ${src})

## data_lib is a parent-tree target and nothing in this library includes a header
## from it; guard the link so a standalone configure does not fail at generate
## time on a non-existent target.
if(TARGET data_lib)
    target_link_libraries(${PROJECT_NAME} PUBLIC data_lib)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE pthread stdc++)

if(TDMS_ENABLE_SANITIZERS)
    set(_tdms_sanitizer_flags -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -g)
    target_compile_options(${PROJECT_NAME} PUBLIC ${_tdms_sanitizer_flags})
    target_link_options(${PROJECT_NAME} PUBLIC ${_tdms_sanitizer_flags})
endif()

if (NOT CMAKE_CROSSCOMPILING)
    if(BUILD_UNIT_TESTS)
        enable_testing()
        add_subdirectory(tests)

        ## Runs the suite as part of the build, immediately after the library and the
        ## test binary are up to date, so a regression fails the build instead of
        ## waiting for someone to remember `ctest`.
        add_custom_target(rp_tdms_lib_run_tests ALL
            ## Run from this library's own binary directory: enable_testing() above
            ## is scoped to this directory, so the test list generated by
            ## gtest_discover_tests lives here and not in the top-level build dir.
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Running tdms_lib unit tests (ctest) - the build fails if any test fails"
            VERBATIM)
        add_dependencies(rp_tdms_lib_run_tests tdms_tests)

    endif()
endif()


file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${headers}
        "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}"
    COMMENT "Copying ${PROJECT_NAME} public headers to ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME}"
    VERBATIM)
