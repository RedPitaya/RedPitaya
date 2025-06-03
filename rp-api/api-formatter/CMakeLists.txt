cmake_minimum_required(VERSION 3.14)
project(rp-formatter)

option(BUILD_SHARED "Builds shared library" ON)
option(BUILD_STATIC "Builds static library" ON)
option(BUILD_PYTHON_MODULE "Builds python module" ON)

option(IS_INSTALL "Install library" ON)
option(BUILD_DOC "Build documentation" ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 20)
set(C_STANDARD 20)


if(NOT DEFINED INSTALL_DIR)
    message(WARNING,"Installation path not set. Installation will be skipped")
    set(IS_INSTALL OFF)
endif()

if(NOT DEFINED DEBUG_REG)
  set(DEBUG_REG OFF)
endif()


if(NOT DEFINED VERSION)
  set(VERSION 0.00-0000)
endif()

if(NOT DEFINED REVISION)
  set(REVISION devbuild)
endif()

if(NOT DEFINED BUILD_NUMBER)
  set(BUILD_NUMBER devbuild)
endif()

message(STATUS "Project=${PROJECT_NAME}")
message(STATUS "VERSION=${VERSION}")
message(STATUS "REVISION=${REVISION}")
message(STATUS "BUILD_NUMBER=${BUILD_NUMBER}")
message(STATUS "LINUX_VER=${LINUX_VER}")
message(STATUS "Is install ${IS_INSTALL}")
message(STATUS "Install path ${INSTALL_DIR}")

message(STATUS "Build shared ${BUILD_SHARED}")
message(STATUS "Build static ${BUILD_STATIC}")

message(STATUS "Compiler C path: ${CMAKE_C_COMPILER}")
message(STATUS "Compiler C ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "Compiler C version: ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "Compiler C is part: ${CMAKE_COMPILER_IS_GNUC}")

message(STATUS "Compiler C++ path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler C++ ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler C++version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Compiler C++ is part: ${CMAKE_COMPILER_IS_GNUCXX}")

list(APPEND r_paths
    /opt/redpitaya/lib
    /opt/redpitaya/lib/web
)

set(CMAKE_INSTALL_RPATH ${r_paths})

list(APPEND src
            ${PROJECT_SOURCE_DIR}/src/writers/rp_csv_writer.cpp
            ${PROJECT_SOURCE_DIR}/src/writers/rp_wav_writer.cpp
            ${PROJECT_SOURCE_DIR}/src/writers/rp_tdms_writer.cpp
            ${PROJECT_SOURCE_DIR}/src/tdms/binary_stream.cpp
            ${PROJECT_SOURCE_DIR}/src/tdms/data_type.cpp
            ${PROJECT_SOURCE_DIR}/src/tdms/file.cpp
            ${PROJECT_SOURCE_DIR}/src/tdms/writer.cpp
            ${PROJECT_SOURCE_DIR}/src/rp_formatter.cpp
        )

list(APPEND header
    ${PROJECT_SOURCE_DIR}/src/rp_formatter.h
)

add_library(${PROJECT_NAME}-obj OBJECT ${src})

target_include_directories(${PROJECT_NAME}-obj PUBLIC "include")
target_include_directories(${PROJECT_NAME}-obj PUBLIC "src")
target_include_directories(${PROJECT_NAME}-obj PUBLIC "src/writers")
target_include_directories(${PROJECT_NAME}-obj PUBLIC "src/tdms")

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    target_compile_options(${PROJECT_NAME}-obj
        PRIVATE -mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC)
    target_compile_definitions(${PROJECT_NAME}-obj
        PRIVATE ARCH_ARM)
endif()

add_compile_options($<$<CONFIG:Debug>:-DTRACE_ENABLE>)
target_compile_options(${PROJECT_NAME}-obj PRIVATE -Wall -Wno-psabi -pedantic -Wextra -Wno-unused-parameter -DVERSION=${VERSION} -DREVISION=${REVISION} $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os> -ffunction-sections -fdata-sections)


if(BUILD_SHARED)

    if (BUILD_PYTHON_MODULE)
        find_package(SWIG REQUIRED)
        INCLUDE(${SWIG_USE_FILE})

        find_package(PythonLibs)

        set_property(SOURCE src/rp_formatter.i PROPERTY CPLUSPLUS ON)
        # SET_SOURCE_FILES_PROPERTIES(src/rp_dsp.i PROPERTIES SWIG_FLAGS "-includeall")

        SWIG_ADD_LIBRARY(rp_formatter_py LANGUAGE python SOURCES src/rp_formatter.i ${src})
        SWIG_LINK_LIBRARIES(rp_formatter_py ${PYTHON_LIBRARIES} -lm -lpthread)
        set_property(TARGET rp_formatter_py PROPERTY INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/src ${PYTHON_INCLUDE_PATH} /usr/local/lib/python3.10/dist-packages/numpy/_core/include)
     endif()

    add_library(${PROJECT_NAME}-shared SHARED)
    set_property(TARGET ${PROJECT_NAME}-shared PROPERTY OUTPUT_NAME ${PROJECT_NAME})
    target_sources(${PROJECT_NAME}-shared PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-obj>)
    target_link_libraries(${PROJECT_NAME}-shared -lm -lpthread)

    if(IS_INSTALL)
        install(TARGETS ${PROJECT_NAME}-shared
            LIBRARY DESTINATION ${INSTALL_DIR}/lib
            ARCHIVE DESTINATION ${INSTALL_DIR}/lib)

        install(FILES ${header}
            DESTINATION ${INSTALL_DIR}/include/common)

        if (BUILD_PYTHON_MODULE)
            install(FILES $<TARGET_PROPERTY:rp_formatter_py,SWIG_SUPPORT_FILES> DESTINATION ${INSTALL_DIR}/lib/python)
            install(TARGETS rp_formatter_py
                LIBRARY DESTINATION ${INSTALL_DIR}/lib/python
                ARCHIVE DESTINATION ${INSTALL_DIR}/lib/python)
            install(FILES tests/rp_formatter_test.py
                DESTINATION ${INSTALL_DIR}/lib/python PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                GROUP_EXECUTE GROUP_READ WORLD_READ WORLD_WRITE WORLD_EXECUTE)
        endif()
    endif()
endif()


if(BUILD_STATIC)
    add_library(${PROJECT_NAME}-static STATIC)
    set_property(TARGET ${PROJECT_NAME}-static PROPERTY OUTPUT_NAME ${PROJECT_NAME})
    target_sources(${PROJECT_NAME}-static PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-obj>)
    target_link_libraries(${PROJECT_NAME}-static -lm -lpthread)

    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}")
    add_custom_command(TARGET ${PROJECT_NAME}-static POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${header}
        "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}"
        COMMENT "Copying ${PROJECT_NAME} public headers to ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME}"
        VERBATIM)

    if(IS_INSTALL)
        install(TARGETS ${PROJECT_NAME}-static
            LIBRARY DESTINATION ${INSTALL_DIR}/lib
            ARCHIVE DESTINATION ${INSTALL_DIR}/lib)

        install(FILES ${header}
            DESTINATION ${INSTALL_DIR}/include/common)
    endif()
endif()
