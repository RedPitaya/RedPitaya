cmake_minimum_required(VERSION 3.18)
project(convert_tool)

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)


message(STATUS "Project=${PROJECT_NAME}")
message(STATUS "RedPitaya platform=${RP_PLATFORM}")
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


set(CMAKE_CXX_STANDARD 17)

FILE(GLOB_RECURSE INC_ALL "*.h")


option(BUILD_PYTHON_MODULE "" OFF)
option(BUILD_SHARED "" OFF)
set(IS_INSTALL_CUR IS_INSTALL)
set(IS_INSTALL OFF CACHE INTERNAL "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../rp-api/api-formatter ${CMAKE_CURRENT_BINARY_DIR}/api-formatter)
set(IS_INSTALL IS_INSTALL_CUR)
add_executable(${PROJECT_NAME} ${INC_ALL})
add_custom_target(install-rp-formatter DEPENDS rp-formatter-static)
add_dependencies(${PROJECT_NAME} install-rp-formatter)

target_link_options(${PROJECT_NAME} PRIVATE --static)

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    target_compile_options(${PROJECT_NAME}
        PRIVATE -mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC -DRP_PLATFORM)

    target_compile_definitions(${PROJECT_NAME}
        PRIVATE ARCH_ARM)
endif()

if (RP_PLATFORM)
    target_compile_options(${PROJECT_NAME} PRIVATE -DRP_PLATFORM)
endif()

target_compile_options(${PROJECT_NAME}
    PRIVATE -std=c++17 -Wall -Wextra -fpermissive  -DVERSION=${VERSION} -DREVISION=${REVISION} $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Debug>:-DTRACE_ENABLE> $<$<CONFIG:Release>:-Os> -ffunction-sections -fdata-sections)

target_include_directories(${PROJECT_NAME}
    PUBLIC  ${PROJECT_SOURCE_DIR}
            ${CMAKE_BINARY_DIR}/bin/include
            )

list(APPEND headers
        )

list(APPEND src
            ${PROJECT_SOURCE_DIR}/src/main.cpp
        )

    target_sources(${PROJECT_NAME} PRIVATE ${src})

target_link_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_BINARY_DIR}/bin/
    ${CMAKE_BINARY_DIR}/lib/
    ${CMAKE_BINARY_DIR}/output/
    )

target_link_libraries(${PROJECT_NAME} PUBLIC converter_lib writer_lib rp-formatter)

if(NOT WIN32 )
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread -static-libstdc++ -static-libgcc)
else()
    set(WIN_SUFFIX -win)
    target_link_libraries(${PROJECT_NAME} PRIVATE -static pthread wsock32 ws2_32)
endif()

if(IS_INSTALL)
        install(TARGETS ${PROJECT_NAME}
              DESTINATION ${INSTALL_DIR}/streaming/convert_tool-${VERSION}-${REVISION}${WIN_SUFFIX}${CLIENT_SUFFIX})
        install(CODE "execute_process(COMMAND zip -urj ${INSTALL_DIR}/streaming/rpsa_client-${VERSION}-${REVISION}${WIN_SUFFIX}${CLIENT_SUFFIX}.zip ${INSTALL_DIR}/streaming/convert_tool-${VERSION}-${REVISION}${WIN_SUFFIX}${CLIENT_SUFFIX})")
        install(CODE "execute_process(COMMAND rm -rf ${INSTALL_DIR}/streaming/convert_tool-${VERSION}-${REVISION}${WIN_SUFFIX}${CLIENT_SUFFIX})")
endif()
