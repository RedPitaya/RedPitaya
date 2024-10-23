cmake_minimum_required(VERSION ${CMAKEVERS})
project(python_lib)

add_custom_target(python_lib_project)

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

if(NOT DEFINED RP_PLATFORM)
  set(RP_PLATFORM OFF)
endif()

if(NOT DEFINED JENKINS_BUILD)
  set(JENKINS_BUILD OFF)
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
message(STATUS "JENKINS_BUILD=${JENKINS_BUILD}")

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

FILE(GLOB_RECURSE INC_ALL "*.h")

find_package(SWIG REQUIRED)
INCLUDE(${SWIG_USE_FILE})

if(NOT JENKINS_BUILD)
find_package(PythonLibs)
endif()

set(CMAKE_SWIG_FLAGS -threads -builtin -O)
set_property(SOURCE src/streaming.i PROPERTY CPLUSPLUS ON)

include_directories(
        ${CMAKE_BINARY_DIR}/bin/include
        ${PROJECT_SOURCE_DIR}/src)

list(APPEND src
        ${PROJECT_SOURCE_DIR}/src/adc_streaming.cpp
        ${PROJECT_SOURCE_DIR}/src/dac_streaming.cpp
        ${PROJECT_SOURCE_DIR}/src/config.cpp
        ${PROJECT_SOURCE_DIR}/src/common.cpp
)

if(WIN32)
    set(WIN_SUFFIX -win)
    set(OS_PATH windows)
else()
    set(OS_PATH linux)
endif()

function(BuildLibrary PyVerName PyVer)

SWIG_ADD_LIBRARY(${PROJECT_NAME}${PyVerName} LANGUAGE python SOURCES src/streaming.i ${src} ${INC_ALL})

if(JENKINS_BUILD)
    target_include_directories(${PROJECT_NAME}${PyVerName} PRIVATE /opt/python/${OS_PATH}/include/python${PyVer})
else()
    target_include_directories(${PROJECT_NAME}${PyVerName} PRIVATE ${PYTHON_INCLUDE_PATH})
endif()

target_compile_options(${PROJECT_NAME}${PyVerName} PRIVATE -Wno-unused-parameter -Wno-unused-variable -Wno-missing-field-initializers)

target_link_directories(${PROJECT_NAME}${PyVerName}
                        PRIVATE
                        ${CMAKE_BINARY_DIR}/bin/
                        ${CMAKE_BINARY_DIR}/lib/
                        )
if(JENKINS_BUILD)
    target_link_directories(${PROJECT_NAME}${PyVerName} PRIVATE /opt/python/${OS_PATH}/lib)
    SWIG_LINK_LIBRARIES(${PROJECT_NAME}${PyVerName} python${PyVer})
else()
    SWIG_LINK_LIBRARIES(${PROJECT_NAME}${PyVerName} ${PYTHON_LIBRARIES})
endif()

if(NOT WIN32)
    SWIG_LINK_LIBRARIES(${PROJECT_NAME}${PyVerName} config_net_lib dac_streaming_lib converter_lib streaming_lib settings_lib data_lib logger_lib pthread -lstdc++ -lgcc)
else()
    SWIG_LINK_LIBRARIES(${PROJECT_NAME}${PyVerName} -static config_net_lib dac_streaming_lib converter_lib streaming_lib settings_lib data_lib logger_lib pthread wsock32 ws2_32)
endif()

if(IS_INSTALL)
    if(JENKINS_BUILD)
        install(FILES src/streaming.py
            DESTINATION ${INSTALL_DIR}/streaming/${PROJECT_NAME}/python PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
            GROUP_EXECUTE GROUP_READ WORLD_READ WORLD_WRITE WORLD_EXECUTE)
    else()
        install(FILES $<TARGET_PROPERTY:${PROJECT_NAME}${PyVerName},SWIG_SUPPORT_FILES> DESTINATION DESTINATION ${INSTALL_DIR}/streaming/${PROJECT_NAME}/python)
    endif()
        install(TARGETS ${PROJECT_NAME}${PyVerName}
            LIBRARY DESTINATION ${INSTALL_DIR}/streaming/${PROJECT_NAME}/python
            ARCHIVE DESTINATION ${INSTALL_DIR}/streaming/${PROJECT_NAME}/python)

        # install(FILES test/streaming_test.py
        #     DESTINATION ${INSTALL_DIR}/streaming/${PROJECT_NAME}/python PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
        #     GROUP_EXECUTE GROUP_READ WORLD_READ WORLD_WRITE WORLD_EXECUTE)

        install(FILES examples/adc_example_1.py examples/adc_example_2.py examples/dac_example_1.py examples/dac_example_2.py examples/dac_example_3.py
            DESTINATION ${INSTALL_DIR}/streaming/${PROJECT_NAME}/python PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
            GROUP_EXECUTE GROUP_READ WORLD_READ WORLD_WRITE WORLD_EXECUTE)

        install(CODE "execute_process(COMMAND zip -urj ${INSTALL_DIR}/streaming/rpsa_client-${VERSION}-${REVISION}${WIN_SUFFIX}${CLIENT_SUFFIX}.zip ${INSTALL_DIR}/streaming/${PROJECT_NAME})")
        install(CODE "execute_process(COMMAND rm -rf ${INSTALL_DIR}/streaming/${PROJECT_NAME})")
endif()

endfunction()

if(JENKINS_BUILD)
    BuildLibrary(3_9 3.9)
    BuildLibrary(3_10 3.10)
    BuildLibrary(3_11 3.11)
    BuildLibrary(3_12 3.12)
    if(WIN32)
        BuildLibrary(3_13 3.13)
    endif()
else()
    BuildLibrary("" "")
endif()





