cmake_minimum_required(VERSION 3.14)
project(rp)

include(CMakeDoxy.txt)

option(BUILD_SHARED "Builds shared library" ON)
option(BUILD_STATIC "Builds static library" ON)
option(IS_INSTALL "Install library" ON)
option(BUILD_DOC "Build documentation" ON)
option(BUILD_PYTHON_MODULE "Builds python module" ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 20)
set(C_STANDARD 20)

if(NOT DEFINED INSTALL_DIR)
    message(FATAL_ERROR,"Installation path not set. Installation will be skipped")
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

message(STATUS "VERSION=${VERSION}")
message(STATUS "REVISION=${REVISION}")
message(STATUS "Install path ${INSTALL_DIR}")
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

list(APPEND r_paths
    /opt/redpitaya/lib
    /opt/redpitaya/lib/web
)

set(CMAKE_INSTALL_RPATH ${r_paths})

include_directories("include")
include_directories("include/common")
include_directories("src")
include_directories(${INSTALL_DIR}/include)


list(APPEND src
            ${CMAKE_SOURCE_DIR}/src/common.cpp
            ${CMAKE_SOURCE_DIR}/src/oscilloscope.cpp
            ${CMAKE_SOURCE_DIR}/src/acq_handler.cpp
            ${CMAKE_SOURCE_DIR}/src/rp.cpp
            ${CMAKE_SOURCE_DIR}/src/generate.cpp
            ${CMAKE_SOURCE_DIR}/src/gen_handler.cpp
            ${CMAKE_SOURCE_DIR}/src/axi_manager.cpp
            ${CMAKE_SOURCE_DIR}/src/housekeeping.cpp
            ${CMAKE_SOURCE_DIR}/src/daisy.cpp
            ${CMAKE_SOURCE_DIR}/src/convert.hpp
            ${CMAKE_SOURCE_DIR}/src/convert_neon.hpp
            ${CMAKE_SOURCE_DIR}/src/profiler.cpp
)


list(APPEND header_rp
    ${CMAKE_SOURCE_DIR}/include/rp.h
    ${CMAKE_SOURCE_DIR}/include/rp_acq.h
    ${CMAKE_SOURCE_DIR}/include/rp_acq_axi.h
    ${CMAKE_SOURCE_DIR}/include/rp_asg_axi.h
    ${CMAKE_SOURCE_DIR}/include/rp_enums.h
    ${CMAKE_SOURCE_DIR}/include/rp_gen.h
)

list(APPEND header_rp_common
    ${CMAKE_SOURCE_DIR}/include/common/version.h
    ${CMAKE_SOURCE_DIR}/include/common/rp_log.h
    ${CMAKE_SOURCE_DIR}/include/common/profiler.h
)


add_compile_options(-mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC)
add_compile_definitions(ARCH_ARM)
add_compile_options(-Wall -Wno-psabi -Wextra -Wno-unused-parameter -DVERSION=${VERSION} -DREVISION=${REVISION} $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Debug>:-DTRACE_ENABLE> $<$<CONFIG:Release>:-Os> -ffunction-sections -fdata-sections)

if(DEBUG_REG)
add_compile_options(-DDEBUG_REG)
endif()

add_library(${PROJECT_NAME}-obj OBJECT ${src})

if(NOT DEFINED INSTALL_DIR)
    message(FATAL_ERROR "You must specify the path to the libraries api-250-12 api-hw-calib api-hw-profiles")
endif()
include_directories(${INSTALL_DIR}/include/api250-12)

link_directories(${INSTALL_DIR}/lib)
link_directories(${INSTALL_DIR}/lib/python)

target_link_libraries(${PROJECT_NAME}-obj rp-i2c)

if(BUILD_SHARED)

    if (BUILD_PYTHON_MODULE)
        find_package(SWIG REQUIRED)
        INCLUDE(${SWIG_USE_FILE})

        find_package(PythonLibs)
        include_directories(${PYTHON_INCLUDE_PATH})
        include_directories(/usr/local/lib/python3.10/dist-packages/numpy/_core/include)
        set(CMAKE_SWIG_FLAGS "")
        set_property(SOURCE src/rp.i PROPERTY CPLUSPLUS ON)

        SWIG_ADD_LIBRARY(rp_py LANGUAGE python SOURCES src/rp.i ${src})
        SWIG_LINK_LIBRARIES(rp_py ${PYTHON_LIBRARIES} -lpthread rp-hw-calib rp-hw-profiles rp-i2c rp-spi rp-gpio rp-hw i2c -lm)
    endif()

    add_library(${PROJECT_NAME}-shared SHARED)
    set_property(TARGET ${PROJECT_NAME}-shared PROPERTY OUTPUT_NAME ${PROJECT_NAME})
    target_sources(${PROJECT_NAME}-shared PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-obj>)
    target_link_libraries(${PROJECT_NAME}-shared rp-hw-calib  rp-hw rp-hw-profiles rp-i2c rp-spi rp-gpio i2c -lm -lpthread)
    if(IS_INSTALL)
        install(TARGETS ${PROJECT_NAME}-shared
            LIBRARY DESTINATION ${INSTALL_DIR}/lib
            ARCHIVE DESTINATION ${INSTALL_DIR}/lib)

        install(FILES ${header_rp}
            DESTINATION ${INSTALL_DIR}/include)

        install(FILES ${header_rp_common}
            DESTINATION ${INSTALL_DIR}/include/common)

        if (BUILD_PYTHON_MODULE)
            install(FILES $<TARGET_PROPERTY:rp_py,SWIG_SUPPORT_FILES> DESTINATION ${INSTALL_DIR}/lib/python)
            install(TARGETS rp_py
                LIBRARY DESTINATION ${INSTALL_DIR}/lib/python
                ARCHIVE DESTINATION ${INSTALL_DIR}/lib/python)
            install(FILES tests/rp_test.py
                DESTINATION ${INSTALL_DIR}/lib/python PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                GROUP_EXECUTE GROUP_READ WORLD_READ WORLD_WRITE WORLD_EXECUTE)
            install(FILES python/rp_overlay.py
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

    if(IS_INSTALL)
        install(TARGETS ${PROJECT_NAME}-static
            LIBRARY DESTINATION ${INSTALL_DIR}/lib
            ARCHIVE DESTINATION ${INSTALL_DIR}/lib)

        install(FILES ${header_rp}
            DESTINATION ${INSTALL_DIR}/include)

        install(FILES ${header_rp_common}
            DESTINATION ${INSTALL_DIR}/include/common)
    endif()
endif()

if(BUILD_DOC)
set(DOXY_OUTPUT_LANGUAGE "English")
set(DOXY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doc")
set(DOXY_INPUT ${header_rp})
ADD_DOCUMENTATION(doc Doxyfile)
endif()

unset(INSTALL_DIR CACHE)
