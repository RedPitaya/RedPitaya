cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 11)
set(C_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

project(monitor)

if(NOT DEFINED MODEL)
  set(MODEL Z10)
endif()

if(NOT DEFINED INSTALL_DIR)
    message(FATAL_ERROR,"Installation path not set.")
endif()

if(NOT DEFINED VERSION)
  set(VERSION 0)
endif()

if(NOT DEFINED REVISION)
  set(REVISION devbuild)
endif()


message(STATUS "RedPitaya model=${MODEL}")
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


include_directories(${INSTALL_DIR}/include)

file(GLOB SOURCES "src/*.cpp")
file(GLOB RP_HEADERS "src/*.h")

add_compile_options(-mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC)
add_compile_definitions(ARCH_ARM)
add_compile_options(-Wall -Wextra -D${MODEL} -DVERSION=${VERSION} -DREVISION=${REVISION} $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os> -ffunction-sections -fdata-sections)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_directories(${PROJECT_NAME} PRIVATE ${INSTALL_DIR}/lib)

target_link_libraries(${PROJECT_NAME} PRIVATE -static rp rp-hw rp-hw-profiles i2c)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${INSTALL_DIR}/bin)

unset(MODEL CACHE)
unset(INSTALL_DIR CACHE)
