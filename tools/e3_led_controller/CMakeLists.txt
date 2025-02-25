cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 20)
set(C_STANDARD 20)
set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

project(e3_controller)
set(e3_led e3_led_controller)
set(e3_i2c e3_i2c_controller)

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

add_compile_options(-mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC)
add_compile_definitions(ARCH_ARM)
add_compile_options($<$<CONFIG:Debug>:-DTRACE_ENABLE>)
add_compile_options(-Wall -pedantic -Wno-unused-result -Wextra -DVERSION=${VERSION} -DREVISION=${REVISION} $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os>  -ffunction-sections -fdata-sections)

add_executable(${e3_led} "./src/e3_led_controller.cpp")
add_executable(${e3_i2c} "./src/e3_i2c_controller.cpp")

target_link_directories(${e3_led} PRIVATE ${INSTALL_DIR}/lib)
target_link_libraries(${e3_led} PRIVATE rp-hw-profiles.a pthread)

target_link_directories(${e3_i2c} PRIVATE ${INSTALL_DIR}/lib)
target_link_libraries(${e3_i2c} PRIVATE rp-hw-profiles.a pthread)

install(TARGETS ${e3_led} RUNTIME DESTINATION ${INSTALL_DIR}/bin)
install(TARGETS ${e3_i2c} RUNTIME DESTINATION ${INSTALL_DIR}/bin)

unset(INSTALL_DIR CACHE)
