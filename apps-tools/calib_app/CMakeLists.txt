cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_VERBOSE_MAKEFILE OFF)

set(APP calib_app)
set(BUILD_PATH ${CMAKE_BINARY_DIR}/${APP})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_PATH})

project(controllerhf)

if(NOT DEFINED INSTALL_DIR)
    message(FATAL_ERROR,"Installation path not set.")
endif()

if(NOT DEFINED VERSION)
    set(VERSION dev)
endif()

if(NOT DEFINED REVISION)
    set(REVISION devbuild)
endif()

if(NOT DEFINED BUILD_NUMBER)
  set(BUILD_NUMBER devbuild)
endif()

message(STATUS "Install path ${INSTALL_DIR}")
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

set(ZIP_FILE ../../../${APP}-${VERSION}-${REVISION}.zip)

list(APPEND r_paths
    /opt/redpitaya/lib
    /opt/redpitaya/lib/web
)

list(APPEND include_paths
    ${INSTALL_DIR}/include
    ${INSTALL_DIR}/include/api2
    ${INSTALL_DIR}/include/apiApp
    ${INSTALL_DIR}/include/api250-12
    ${INSTALL_DIR}/include/redpitaya
    ${INSTALL_DIR}/include/common
    ${INSTALL_DIR}/include/calib
    ${INSTALL_DIR}/rp_sdk
    ${INSTALL_DIR}/rp_sdk/libjson
)

list(APPEND libs_paths
    ${INSTALL_DIR}/lib
    ${INSTALL_DIR}/lib/web
    ${INSTALL_DIR}/rp_sdk
)

list(APPEND rp_libs
    rp-system
    rp-client
    rpapp
    rpapp_lcr
    rp
    rp-arb
    rp-dsp
    rp-sweep
    rp-formatter
    rp-calib
    rp-hw-calib
    rp-hw-can
    rp-hw
    rp-i2c
    rp-spi
    rp-gpio
    i2c
)

set(CMAKE_INSTALL_RPATH ${r_paths})
set(CMAKE_BUILD_RPATH ${r_paths})
include_directories(${include_paths})
link_directories(${libs_paths})

file(GLOB SOURCES "src/*.cpp")

add_compile_options(-mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC)
add_compile_definitions(ARCH_ARM)
add_compile_options(-Wall -Wno-psabi -Wno-reorder -Wno-cpp -Wextra $<$<CONFIG:Release>:-Wno-unused-parameter> $<$<CONFIG:Release>:-Wno-unused-variable>)
add_compile_options($<$<CONFIG:Debug>:-DTRACE_ENABLE>)
add_compile_options($<$<CONFIG:Debug>:-DZIP_DISABLED>)
add_compile_options(-DVERSION=${VERSION} -DREVISION=${REVISION} $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os>  -ffunction-sections -fdata-sections)


add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE cryptopp rp_sdk ${rp_libs} pthread)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")


install(DIRECTORY ${CMAKE_SOURCE_DIR}/css DESTINATION ${BUILD_PATH})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/img DESTINATION ${BUILD_PATH})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/info DESTINATION ${BUILD_PATH})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/js DESTINATION ${BUILD_PATH})

file(GLOB HTML "*.html")
install(FILES ${HTML} DESTINATION ${BUILD_PATH})
install(FILES nginx.conf fpga.conf fpga.sh DESTINATION ${BUILD_PATH})

install(CODE "execute_process(COMMAND sed -i ${BUILD_PATH}/info/info.json -e \"s/REVISION/${REVISION}/\")")
install(CODE "execute_process(COMMAND sed -i ${BUILD_PATH}/info/info.json -e \"s/VERSION/${VERSION}/\")")

install(CODE "file(ARCHIVE_CREATE OUTPUT ${ZIP_FILE} FORMAT zip PATHS ${BUILD_PATH})")
install(CODE "file(ARCHIVE_EXTRACT INPUT ${ZIP_FILE} DESTINATION ${INSTALL_DIR}/www/apps)")
