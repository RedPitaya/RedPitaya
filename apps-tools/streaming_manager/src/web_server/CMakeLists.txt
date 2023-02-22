cmake_minimum_required(VERSION 3.18)
project(web_server)

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

message(STATUS "Project=${PROJECT_NAME}")
message(STATUS "RedPitaya platform=${RP_PLATFORM}")
message(STATUS "Is install ${IS_INSTALL}")
message(STATUS "Install path ${INSTALL_DIR}")
message(STATUS "VERSION=${VERSION}")
message(STATUS "REVISION=${REVISION}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}")

message(STATUS "Compiler С path: ${CMAKE_C_COMPILER}")
message(STATUS "Compiler С ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "Compiler С version: ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "Compiler С is part: ${CMAKE_COMPILER_IS_GNUC}")

message(STATUS "Compiler С++ path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler С++ ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler С++version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Compiler С++ is part: ${CMAKE_COMPILER_IS_GNUCXX}")


set(CMAKE_CXX_STANDARD 14)

FILE(GLOB_RECURSE INC_ALL "*.h")

add_library(${PROJECT_NAME} SHARED ${INC_ALL})

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    target_compile_options(${PROJECT_NAME}
        PRIVATE -mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC -DRP_PLATFORM)

    target_compile_definitions(${PROJECT_NAME}
        PRIVATE ARCH_ARM)
endif()

if (RP_PLATFORM)
    target_compile_options(${PROJECT_NAME} PRIVATE -DRP_PLATFORM)
    target_include_directories(${PROJECT_NAME} PUBLIC ${INSTALL_DIR}/include ${INSTALL_DIR}/rp_sdk ${INSTALL_DIR}/rp_sdk/libjson)
    target_link_directories(${PROJECT_NAME} PRIVATE ${INSTALL_DIR}/lib/ ${INSTALL_DIR}/rp_sdk)
    target_link_libraries(${PROJECT_NAME} PUBLIC -Wl,--as-needed rp rp-hw rp_sdk rp-hw rp-hw-calib rp-hw-profiles -Wl,--no-as-needed)
    target_link_libraries(${PROJECT_NAME} PRIVATE rp-i2c rp-spi rp-gpio i2c)
endif()

target_compile_options(${PROJECT_NAME}
    PRIVATE -Wall -fpermissive -DVERSION=${VERSION} -DREVISION=${REVISION} $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os -s>  -ffunction-sections -fdata-sections)

target_include_directories(${PROJECT_NAME}
    PUBLIC  ${PROJECT_SOURCE_DIR}
            ${CMAKE_BINARY_DIR}/bin/include
            )

list(APPEND headers
            ${PROJECT_SOURCE_DIR}/src/main.h
        )

list(APPEND src
            ${PROJECT_SOURCE_DIR}/src/main.cpp
        )

    target_sources(${PROJECT_NAME} PRIVATE ${src})

target_link_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_BINARY_DIR}/bin/
    ${CMAKE_BINARY_DIR}/lib/
    )


target_link_libraries(${PROJECT_NAME} PUBLIC config_net_lib dac_streaming_lib streaming_lib settings_lib pthread stdc++)

if(IS_INSTALL)
       install(TARGETS ${PROJECT_NAME}
              DESTINATION ${INSTALL_DIR}/bin)

endif()
