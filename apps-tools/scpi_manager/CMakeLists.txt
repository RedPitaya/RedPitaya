cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE OFF)

set(APP scpi_manager)
set(BUILD_PATH ${CMAKE_BINARY_DIR}/${APP})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_PATH})

project(controllerhf)

if(NOT DEFINED INSTALL_DIR)
    message(FATAL_ERROR,"Installation path not set.")
endif()

if(NOT DEFINED VERSION)
  set(VERSION 0)
endif()

if(NOT DEFINED REVISION)
  set(REVISION devbuild)
endif()


message(STATUS "Install path ${INSTALL_DIR}")
message(STATUS "VERSION=${VERSION}")
message(STATUS "REVISION=${REVISION}")

message(STATUS "Compiler C path: ${CMAKE_C_COMPILER}")
message(STATUS "Compiler C ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "Compiler C version: ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "Compiler C is part: ${CMAKE_COMPILER_IS_GNUC}")

message(STATUS "Compiler C++ path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler C++ ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler C++version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Compiler C++ is part: ${CMAKE_COMPILER_IS_GNUCXX}")

set(ZIP_FILE ../../../${APP}-${VERSION}-${BUILD_NUMBER}-${REVISION}.zip)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/css DESTINATION ${BUILD_PATH})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/img DESTINATION ${BUILD_PATH})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/info DESTINATION ${BUILD_PATH})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/js DESTINATION ${BUILD_PATH})

file(GLOB HTML "*.html")
install(FILES ${HTML} DESTINATION ${BUILD_PATH})
install(FILES nginx.conf fpga.conf fpga.sh DESTINATION ${BUILD_PATH})

install(CODE "execute_process(COMMAND sed -i ${BUILD_PATH}/info/info.json -e \"s/REVISION/${REVISION}/\")")
install(CODE "execute_process(COMMAND sed -i ${BUILD_PATH}/info/info.json -e \"s/BUILD_NUMBER/${BUILD_NUMBER}/\")")

install(CODE "file(ARCHIVE_CREATE OUTPUT ${ZIP_FILE} FORMAT zip PATHS ${BUILD_PATH})")
install(CODE "file(ARCHIVE_EXTRACT INPUT ${ZIP_FILE} DESTINATION ${INSTALL_DIR}/www/apps)")
