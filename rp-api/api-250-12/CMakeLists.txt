cmake_minimum_required(VERSION 3.14)


option(BUILD_SHARED "Builds shared library" ON)
option(BUILD_STATIC "Builds static library" ON)
option(BUILD_PYTHON_MODULE "Builds python module" ON)

option(BUILD_TEST   "Build test" OFF)
option(BUILD_TOOL   "Build tool" ON)

option(IS_INSTALL "Install library" ON)

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 11)
set(C_STANDARD 11)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

project(rp)

if(NOT DEFINED INSTALL_DIR)
    message(WARNING,"Installation path not set. Installation will be skipped")
    set(IS_INSTALL OFF)
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


include_directories("src/xml")
include_directories("include")
include_directories("include")
include_directories(${INSTALL_DIR}/include)

file(GLOB PR_XML_SOURCES "src/xml/*.cpp")

file(GLOB PR_I2C_SOURCES "src/rp-i2c.cpp"
                         "src/rp-i2c-mcp47x6.cpp"
                         "src/rp-i2c-max7311.cpp"
                         "src/rp-i2c-mcp47x6-c.cpp"
                         "src/rp-i2c-max7311-c.cpp")

file(GLOB PR_SPI_SOURCES "src/rp-spi.cpp"
                         "src/spi/spi.cpp")


file(GLOB PR_GPIO_SOURCES "src/rp-gpio-power.cpp"
                          "src/gpio/gpio.cpp")

file(GLOB RP_HEADERS "include/*.h")

add_compile_options(-mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC)
add_compile_definitions(ARCH_ARM)
add_compile_options(-Wall -pedantic -Wextra -DVERSION=${VERSION} -DREVISION=${REVISION} $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os> -ffunction-sections -fdata-sections)
link_libraries(-L${INSTALL_DIR}/lib librp-hw.a)

add_library(${PROJECT_NAME}-xml OBJECT ${PR_XML_SOURCES})
add_library(${PROJECT_NAME}-i2c OBJECT ${PR_I2C_SOURCES})
add_library(${PROJECT_NAME}-spi OBJECT ${PR_SPI_SOURCES})
add_library(${PROJECT_NAME}-gpio OBJECT ${PR_GPIO_SOURCES})

if(BUILD_SHARED)

    if (BUILD_PYTHON_MODULE)
        find_package(SWIG REQUIRED)
        INCLUDE(${SWIG_USE_FILE})

        find_package(PythonLibs)
        include_directories(${PYTHON_INCLUDE_PATH})

        set_property(SOURCE src/rp_api_250.i PROPERTY CPLUSPLUS ON)

        SWIG_ADD_LIBRARY(rp_api_250_py LANGUAGE python SOURCES src/rp_api_250.i ${PR_XML_SOURCES} ${PR_I2C_SOURCES} ${PR_SPI_SOURCES} ${PR_GPIO_SOURCES})
        SWIG_LINK_LIBRARIES(rp_api_250_py ${PYTHON_LIBRARIES} i2c)
    endif()

    add_library(${PROJECT_NAME}-i2c-shared SHARED)
    set_property(TARGET ${PROJECT_NAME}-i2c-shared PROPERTY OUTPUT_NAME ${PROJECT_NAME}-i2c)
    target_sources(${PROJECT_NAME}-i2c-shared PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-xml> $<TARGET_OBJECTS:${PROJECT_NAME}-i2c>)
    target_link_libraries(${PROJECT_NAME}-i2c-shared i2c)

    add_library(${PROJECT_NAME}-spi-shared SHARED)
    set_property(TARGET ${PROJECT_NAME}-spi-shared PROPERTY OUTPUT_NAME ${PROJECT_NAME}-spi)
    target_sources(${PROJECT_NAME}-spi-shared PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-xml> $<TARGET_OBJECTS:${PROJECT_NAME}-spi>)

    add_library(${PROJECT_NAME}-gpio-shared SHARED)
    set_property(TARGET ${PROJECT_NAME}-gpio-shared PROPERTY OUTPUT_NAME ${PROJECT_NAME}-gpio)
    target_sources(${PROJECT_NAME}-gpio-shared PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-gpio>)

    if(IS_INSTALL)
        install(TARGETS ${PROJECT_NAME}-i2c-shared
            LIBRARY DESTINATION ${INSTALL_DIR}/lib
            ARCHIVE DESTINATION ${INSTALL_DIR}/lib)

        install(TARGETS ${PROJECT_NAME}-spi-shared
            LIBRARY DESTINATION ${INSTALL_DIR}/lib
            ARCHIVE DESTINATION ${INSTALL_DIR}/lib)

        install(TARGETS ${PROJECT_NAME}-gpio-shared
            LIBRARY DESTINATION ${INSTALL_DIR}/lib
            ARCHIVE DESTINATION ${INSTALL_DIR}/lib)

        install(FILES ${RP_HEADERS}
            DESTINATION ${INSTALL_DIR}/include/api250-12)

        install(DIRECTORY ${CMAKE_SOURCE_DIR}/configs
            DESTINATION ${INSTALL_DIR}/lib FILES_MATCHING PATTERN "*.xml")

        if (BUILD_PYTHON_MODULE)
            install(FILES $<TARGET_PROPERTY:rp_api_250_py,SWIG_SUPPORT_FILES> DESTINATION ${INSTALL_DIR}/lib/python)
            install(TARGETS rp_api_250_py
                LIBRARY DESTINATION ${INSTALL_DIR}/lib/python
                ARCHIVE DESTINATION ${INSTALL_DIR}/lib/python)
            install(FILES test/rp_api_250_test.py
                DESTINATION ${INSTALL_DIR}/lib/python PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                GROUP_EXECUTE GROUP_READ WORLD_READ WORLD_WRITE WORLD_EXECUTE)
        endif()
    endif()
endif()


if(BUILD_STATIC)

    add_library(${PROJECT_NAME}-i2c-static STATIC)
    set_property(TARGET ${PROJECT_NAME}-i2c-static PROPERTY OUTPUT_NAME ${PROJECT_NAME}-i2c)
    target_sources(${PROJECT_NAME}-i2c-static PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-xml> $<TARGET_OBJECTS:${PROJECT_NAME}-i2c>)

    add_library(${PROJECT_NAME}-spi-static STATIC)
    set_property(TARGET ${PROJECT_NAME}-spi-static PROPERTY OUTPUT_NAME ${PROJECT_NAME}-spi)
    target_sources(${PROJECT_NAME}-spi-static PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-xml> $<TARGET_OBJECTS:${PROJECT_NAME}-spi>)

    add_library(${PROJECT_NAME}-gpio-static STATIC)
    set_property(TARGET ${PROJECT_NAME}-gpio-static PROPERTY OUTPUT_NAME ${PROJECT_NAME}-gpio)
    target_sources(${PROJECT_NAME}-gpio-static PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-gpio>)


    if(IS_INSTALL)
        install(TARGETS ${PROJECT_NAME}-i2c-static
            LIBRARY DESTINATION ${INSTALL_DIR}/lib
            ARCHIVE DESTINATION ${INSTALL_DIR}/lib)

        install(TARGETS ${PROJECT_NAME}-spi-static
            LIBRARY DESTINATION ${INSTALL_DIR}/lib
            ARCHIVE DESTINATION ${INSTALL_DIR}/lib)

        install(TARGETS ${PROJECT_NAME}-gpio-static
            LIBRARY DESTINATION ${INSTALL_DIR}/lib
            ARCHIVE DESTINATION ${INSTALL_DIR}/lib)

        install(FILES ${RP_HEADERS}
            DESTINATION ${INSTALL_DIR}/include/api250-12)

        install(DIRECTORY ${CMAKE_SOURCE_DIR}/configs
            DESTINATION ${INSTALL_DIR}/lib FILES_MATCHING PATTERN "*.xml")
    endif()
endif()

if(BUILD_TEST)
    add_executable(rp_i2c_test ${CMAKE_SOURCE_DIR}/test/main.cpp)
    target_link_libraries(rp_i2c_test PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-xml> $<TARGET_OBJECTS:${PROJECT_NAME}-i2c> $<TARGET_OBJECTS:${PROJECT_NAME}-spi>  $<TARGET_OBJECTS:${PROJECT_NAME}-gpio> i2c)
endif()

if(BUILD_TOOL)
    add_executable(rp_i2c_tool ${CMAKE_SOURCE_DIR}/src/tool/main.cpp)
    target_link_libraries(rp_i2c_tool PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-xml> $<TARGET_OBJECTS:${PROJECT_NAME}-i2c> -li2c)

    add_executable(rp_power_on ${CMAKE_SOURCE_DIR}/src/power_on/main.cpp)
    target_link_libraries(rp_power_on PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-xml> $<TARGET_OBJECTS:${PROJECT_NAME}-spi> $<TARGET_OBJECTS:${PROJECT_NAME}-gpio> $<TARGET_OBJECTS:${PROJECT_NAME}-i2c> -li2c)

    if(IS_INSTALL)
        install(TARGETS rp_i2c_tool
            RUNTIME DESTINATION ${INSTALL_DIR}/bin)
        install(TARGETS rp_power_on
            RUNTIME DESTINATION ${INSTALL_DIR}/bin)
    endif()
endif()

unset(INSTALL_DIR CACHE)
