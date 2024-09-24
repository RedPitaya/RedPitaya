cmake_minimum_required(VERSION ${CMAKEVERS})
project(python_lib)

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

if(NOT DEFINED RP_PLATFORM)
  set(RP_PLATFORM OFF)
endif()

message(STATUS "${Yellow}Project=${PROJECT_NAME}${ColourReset}")
message(STATUS "RedPitaya platform=${RP_PLATFORM}")
message(STATUS "CPU ARCH=${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Is install ${IS_INSTALL}")
message(STATUS "Install path ${INSTALL_DIR}")
message(STATUS "VERSION=${VERSION}")
message(STATUS "REVISION=${REVISION}")
message(STATUS "BUILD_NUMBER=${BUILD_NUMBER}")
message(STATUS "CLIENT_SUFFIX=${CLIENT_SUFFIX}")

message(STATUS "Compiler С path: ${CMAKE_C_COMPILER}")
message(STATUS "Compiler С ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "Compiler С version: ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "Compiler С is part: ${CMAKE_COMPILER_IS_GNUC}")

message(STATUS "Compiler С++ path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler С++ ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler С++version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Compiler С++ is part: ${CMAKE_COMPILER_IS_GNUCXX}")
message(STATUS "CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message(STATUS "debug mode")
endif()

set(CMAKE_CXX_STANDARD 20)

FILE(GLOB_RECURSE INC_ALL "*.h")

find_package(SWIG REQUIRED)
INCLUDE(${SWIG_USE_FILE})
find_package(PythonLibs)

set_property(SOURCE src/streaming.i PROPERTY CPLUSPLUS ON)

set(CMAKE_SWIG_FLAGS -threads)

SWIG_ADD_LIBRARY(${PROJECT_NAME} LANGUAGE python SOURCES src/streaming.i ${src})

list(APPEND src
            ${PROJECT_SOURCE_DIR}/src/adc_streaming.cpp
            ${PROJECT_SOURCE_DIR}/src/common.cpp
        )

include_directories(${PYTHON_INCLUDE_PATH}
                    ${CMAKE_BINARY_DIR}/bin/include
                    ${PROJECT_SOURCE_DIR}/src)

target_compile_options(${PROJECT_NAME} PRIVATE -Wno-unused-parameter)

target_sources(${PROJECT_NAME} PRIVATE ${src})

target_link_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_BINARY_DIR}/bin/
    ${CMAKE_BINARY_DIR}/lib/
    )

if(NOT WIN32)
    SWIG_LINK_LIBRARIES(${PROJECT_NAME} ${PYTHON_LIBRARIES} config_net_lib dac_streaming_lib converter_lib streaming_lib settings_lib data_lib logger_lib pthread -lstdc++ -lgcc)
else()
    set(WIN_SUFFIX -win)
    SWIG_LINK_LIBRARIES(${PROJECT_NAME} ${PYTHON_LIBRARIES} -static config_net_lib dac_streaming_lib converter_lib streaming_lib settings_lib data_lib logger_lib pthread wsock32 ws2_32)
endif()

if(IS_INSTALL)
        install(FILES $<TARGET_PROPERTY:${PROJECT_NAME},SWIG_SUPPORT_FILES> DESTINATION DESTINATION ${INSTALL_DIR}/streaming/${PROJECT_NAME})
        install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION ${INSTALL_DIR}/streaming/${PROJECT_NAME}
            ARCHIVE DESTINATION ${INSTALL_DIR}/streaming/${PROJECT_NAME})
        install(FILES test/streaming_test.py
        DESTINATION ${INSTALL_DIR}/streaming/${PROJECT_NAME} PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
        GROUP_EXECUTE GROUP_READ WORLD_READ WORLD_WRITE WORLD_EXECUTE)
        install(CODE "execute_process(COMMAND zip -urj ${INSTALL_DIR}/streaming/rpsa_client-${VERSION}-${REVISION}${WIN_SUFFIX}${CLIENT_SUFFIX}.zip ${INSTALL_DIR}/streaming/${PROJECT_NAME})")
        install(CODE "execute_process(COMMAND rm -rf ${INSTALL_DIR}/streaming/${PROJECT_NAME})")
endif()
