cmake_minimum_required(VERSION 3.14)
project(rpApplications)

option(BUILD_SHARED "Builds shared library" ON)
option(BUILD_STATIC "Builds static library" ON)
option(IS_INSTALL "Install library" ON)
option(ENABLE_LCR "LCR api" ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 17)
set(C_STANDARD 17)


if(NOT DEFINED INSTALL_DIR)
    message(WARNING,"Installation path not set. Installation will be skipped")
    set(IS_INSTALL OFF)
endif()

if(NOT DEFINED VERSION)
  set(VERSION 0.00-0000)
endif()

if(NOT DEFINED REVISION)
  set(REVISION devbuild)
endif()


message(STATUS "VERSION=${VERSION}")
message(STATUS "REVISION=${REVISION}")
message(STATUS "Install path ${INSTALL_DIR}")

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

include_directories(${INSTALL_DIR}/include)
include_directories(${INSTALL_DIR}/include/common)
include_directories(${INSTALL_DIR}/include/api250-12)
include_directories(${CMAKE_SOURCE_DIR}/src)
link_directories(${INSTALL_DIR}/lib)

list(APPEND src_rpapp
        ${CMAKE_SOURCE_DIR}/src/rpApp.cpp
        ${CMAKE_SOURCE_DIR}/src/common.cpp
        ${CMAKE_SOURCE_DIR}/src/osciloscopeApp.cpp
        ${CMAKE_SOURCE_DIR}/src/bodeApp.cpp
        ${CMAKE_SOURCE_DIR}/src/spectrometerApp.cpp
        ${CMAKE_SOURCE_DIR}/src/osciloscope_logic/data_decimator.cpp
        ${CMAKE_SOURCE_DIR}/src/osciloscope_logic/view_controller.cpp
        ${CMAKE_SOURCE_DIR}/src/osciloscope_logic/measure_controller.cpp
        ${CMAKE_SOURCE_DIR}/src/osciloscope_logic/adc_controller.cpp
        )

list(APPEND header_rpapp
        ${CMAKE_SOURCE_DIR}/src/rpApp.h
        ${CMAKE_SOURCE_DIR}/src/bodeApp.h
        )

list(APPEND src_rpapp_lcr
        ${CMAKE_SOURCE_DIR}/lcr_meter/src/calib.cpp
        ${CMAKE_SOURCE_DIR}/lcr_meter/src/lcr_meter.cpp
        ${CMAKE_SOURCE_DIR}/lcr_meter/src/lcr_generator.cpp
        ${CMAKE_SOURCE_DIR}/lcr_meter/src/lcr_hw.cpp
        ${CMAKE_SOURCE_DIR}/lcr_meter/src/lcrApp.cpp
        ${CMAKE_SOURCE_DIR}/lcr_meter/src/utils.cpp
        )

list(APPEND header_rpapp_lcr
        ${CMAKE_SOURCE_DIR}/lcr_meter/src/lcrApp.h
        )

add_compile_options(-mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC)
add_compile_definitions(ARCH_ARM)
add_compile_options(-Wall -pedantic -Wextra -Wno-unused-parameter -DVERSION=${VERSION} -DREVISION=${REVISION} $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Debug>:-DTRACE_ENABLE> $<$<CONFIG:Release>:-Os> -ffunction-sections -fdata-sections)

add_library(rpapp-obj OBJECT ${src_rpapp})

if (ENABLE_LCR)
    add_library(rpapp_lcr-obj OBJECT ${src_rpapp_lcr})
endif()

if(BUILD_SHARED)
    add_library(rpapp-shared SHARED)
    set_property(TARGET rpapp-shared PROPERTY OUTPUT_NAME rpapp)
    target_sources(rpapp-shared PRIVATE $<TARGET_OBJECTS:rpapp-obj>)
    target_link_options(rpapp-shared PRIVATE -shared -Wl,--version-script=${CMAKE_SOURCE_DIR}/src/exportmap)
    target_link_libraries(rpapp-shared -lrp -lm -lpthread)
    target_link_libraries(rpapp-shared rp-hw-calib rp-hw-profiles rp-dsp rp-i2c rp-spi rp-gpio i2c)

    if(IS_INSTALL)
        install(TARGETS rpapp-shared
            LIBRARY DESTINATION ${INSTALL_DIR}/lib
            ARCHIVE DESTINATION ${INSTALL_DIR}/lib)

        install(FILES ${header_rpapp}
            DESTINATION ${INSTALL_DIR}/include/apiApp)
    endif()
endif()


if(BUILD_STATIC)
    add_library(rpapp-static STATIC)
    set_property(TARGET rpapp-static PROPERTY OUTPUT_NAME rpapp)
    target_sources(rpapp-static PRIVATE $<TARGET_OBJECTS:rpapp-obj>)
    target_link_libraries(rpapp-static -lm -lpthread)

    if(IS_INSTALL)
        install(TARGETS rpapp-static
            LIBRARY DESTINATION ${INSTALL_DIR}/lib
            ARCHIVE DESTINATION ${INSTALL_DIR}/lib)

        install(FILES ${header_rpapp}
            DESTINATION ${INSTALL_DIR}/include/apiApp)
    endif()
endif()

if (ENABLE_LCR)
    if(BUILD_SHARED)
        add_library(rpapp_lcr-shared SHARED)
        set_property(TARGET rpapp_lcr-shared PROPERTY OUTPUT_NAME rpapp_lcr)
        target_sources(rpapp_lcr-shared PRIVATE $<TARGET_OBJECTS:rpapp_lcr-obj>)
        target_link_options(rpapp_lcr-shared PRIVATE -shared -Wl,--version-script=${CMAKE_SOURCE_DIR}/lcr_meter/src/exportmap)
        target_link_libraries(rpapp_lcr-shared -lrp -lm -lpthread)
        target_link_libraries(rpapp_lcr-shared rp-dsp rp-hw-calib rp-hw-profiles rp-i2c rp-spi rp-gpio i2c)

        if(IS_INSTALL)
            install(TARGETS rpapp_lcr-shared
                LIBRARY DESTINATION ${INSTALL_DIR}/lib
                ARCHIVE DESTINATION ${INSTALL_DIR}/lib)

            install(FILES ${header_rpapp_lcr}
                DESTINATION ${INSTALL_DIR}/include/apiApp)
        endif()
    endif()


    if(BUILD_STATIC)
        add_library(rpapp_lcr-static STATIC)
        set_property(TARGET rpapp_lcr-static PROPERTY OUTPUT_NAME rpapp_lcr)
        target_sources(rpapp_lcr-static PRIVATE $<TARGET_OBJECTS:rpapp_lcr-obj>)
        target_link_libraries(rpapp_lcr-static -lrp-dsp -lrp -lrp-i2c -lrp-spi -lrp-gpio -li2c -lm -lpthread)

        if(IS_INSTALL)
            install(TARGETS rpapp_lcr-static
                LIBRARY DESTINATION ${INSTALL_DIR}/lib
                ARCHIVE DESTINATION ${INSTALL_DIR}/lib)

            install(FILES ${header_rpapp_lcr}
                DESTINATION ${INSTALL_DIR}/include/apiApp)
        endif()
    endif()
endif()



unset(INSTALL_DIR CACHE)