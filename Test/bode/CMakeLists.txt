cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 11)
set(C_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

project(bode)

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
include_directories(${INSTALL_DIR}/include/apiApp)

file(GLOB SOURCES "src/*.cpp")
file(GLOB RP_HEADERS "src/*.h")

add_compile_options(-mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC)
add_compile_definitions(ARCH_ARM)
add_compile_options(-Wall -pedantic -Wextra -DVERSION=${VERSION} -DREVISION=${REVISION} $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os>  -ffunction-sections -fdata-sections)


add_executable(${PROJECT_NAME} ${SOURCES})

target_link_directories(${PROJECT_NAME} PRIVATE ${INSTALL_DIR}/lib)

target_link_libraries(${PROJECT_NAME} PRIVATE rpapp rp-dsp rp rp-hw rp-hw-calib rp-hw-profiles)

target_link_libraries(${PROJECT_NAME} PRIVATE rp-i2c rp-spi rp-gpio)

target_link_libraries(${PROJECT_NAME} PRIVATE rp-hw i2c)

target_link_libraries(${PROJECT_NAME} PRIVATE pthread)


install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${INSTALL_DIR}/bin)

unset(INSTALL_DIR CACHE)
