cmake_minimum_required(VERSION 3.18)
project(rpsa_client)

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)



if(NOT DEFINED RP_PLATFORM)
  set(RP_PLATFORM OFF)
endif()

message(STATUS "Project=${PROJECT_NAME}")
message(STATUS "RedPitaya platform=${RP_PLATFORM}")
message(STATUS "Is install ${IS_INSTALL}")
message(STATUS "Install path ${INSTALL_DIR}")
message(STATUS "VERSION=${VERSION}")
message(STATUS "REVISION=${REVISION}")
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

add_executable(${PROJECT_NAME} ${INC_ALL})

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    target_compile_options(${PROJECT_NAME}
        PRIVATE -mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC)

    target_compile_definitions(${PROJECT_NAME}
        PRIVATE ARCH_ARM)
endif()

if (RP_PLATFORM)
    target_compile_options(${PROJECT_NAME} PRIVATE -DRP_PLATFORM)
endif()

target_compile_options(${PROJECT_NAME}
    PRIVATE -std=c++17 -Wall -Wextra -fpermissive -DVERSION=${VERSION} -DREVISION=${REVISION} $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os> -ffunction-sections -fdata-sections)

target_link_options(${PROJECT_NAME} PRIVATE --static)

target_include_directories(${PROJECT_NAME}
    PUBLIC  ${PROJECT_SOURCE_DIR}
            ${CMAKE_BINARY_DIR}/bin/include
            )

list(APPEND headers
            ${PROJECT_SOURCE_DIR}/src/config.h
            ${PROJECT_SOURCE_DIR}/src/dac_streaming.h
            ${PROJECT_SOURCE_DIR}/src/main.h
            ${PROJECT_SOURCE_DIR}/src/options.h
            ${PROJECT_SOURCE_DIR}/src/remote.h
            ${PROJECT_SOURCE_DIR}/src/search.h
            ${PROJECT_SOURCE_DIR}/src/streaming.h
            ${PROJECT_SOURCE_DIR}/src/test_helper.h
        )

list(APPEND src
            ${PROJECT_SOURCE_DIR}/src/config.cpp
            ${PROJECT_SOURCE_DIR}/src/dac_streaming.cpp
            ${PROJECT_SOURCE_DIR}/src/main.cpp
            ${PROJECT_SOURCE_DIR}/src/options.cpp
            ${PROJECT_SOURCE_DIR}/src/remote.cpp
            ${PROJECT_SOURCE_DIR}/src/search.cpp
            ${PROJECT_SOURCE_DIR}/src/streaming.cpp
            ${PROJECT_SOURCE_DIR}/src/test_helper.cpp
        )

    target_sources(${PROJECT_NAME} PRIVATE ${src})

target_link_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_BINARY_DIR}/bin/
    ${CMAKE_BINARY_DIR}/lib/
    )

target_link_libraries(${PROJECT_NAME} PUBLIC config_net_lib dac_streaming_lib converter_lib streaming_lib settings_lib)

if(NOT WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread -static-libstdc++ -static-libgcc)
else()
    set(WIN_SUFFIX -win)
    target_link_libraries(${PROJECT_NAME} PRIVATE -static pthread wsock32 ws2_32)
endif()

if(IS_INSTALL)
       install(TARGETS ${PROJECT_NAME}
              DESTINATION ${INSTALL_DIR}/streaming/rpsa_client-${VERSION}-${REVISION}${WIN_SUFFIX}${CLIENT_SUFFIX})

       file(GLOB_RECURSE JSON_FILES
            "${PROJECT_SOURCE_DIR}/*.json"
           )

       install(FILES ${JSON_FILES}
              DESTINATION ${INSTALL_DIR}/streaming/rpsa_client-${VERSION}-${REVISION}${WIN_SUFFIX}${CLIENT_SUFFIX})

        install(CODE "execute_process(COMMAND zip -urj ${INSTALL_DIR}/streaming/rpsa_client-${VERSION}-${REVISION}${WIN_SUFFIX}${CLIENT_SUFFIX}.zip ${INSTALL_DIR}/streaming/rpsa_client-${VERSION}-${REVISION}${WIN_SUFFIX}${CLIENT_SUFFIX})")
        install(CODE "execute_process(COMMAND rm -rf ${INSTALL_DIR}/streaming/rpsa_client-${VERSION}-${REVISION}${WIN_SUFFIX}${CLIENT_SUFFIX})")
endif()
