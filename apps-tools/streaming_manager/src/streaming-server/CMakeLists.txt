cmake_minimum_required(VERSION ${CMAKEVERS})
project(streaming-server)

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

message(STATUS "${Yellow}Project=${PROJECT_NAME}${ColourReset}")
message(STATUS "RedPitaya platform=${RP_PLATFORM}")
message(STATUS "Is install ${IS_INSTALL}")
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
set(CMAKE_BUILD_RPATH ${r_paths})

FILE(GLOB_RECURSE INC_ALL "*.h")

add_executable(${PROJECT_NAME} ${INC_ALL})

if (RP_PLATFORM)
    target_compile_options(${PROJECT_NAME} PRIVATE -DRP_PLATFORM)
    target_include_directories(${PROJECT_NAME} PUBLIC ${INSTALL_DIR}/include)
    target_link_directories(${PROJECT_NAME} PRIVATE ${INSTALL_DIR}/lib/)
    target_link_libraries(${PROJECT_NAME} PUBLIC rp rp-hw rp-hw-calib rp-hw-profiles)
    target_link_libraries(${PROJECT_NAME} PUBLIC rp-i2c rp-spi rp-gpio i2c)
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC  ${PROJECT_SOURCE_DIR}
            ${CMAKE_BINARY_DIR}/bin/include
            )

list(APPEND headers
            ${PROJECT_SOURCE_DIR}/src/dac_streaming.h
            ${PROJECT_SOURCE_DIR}/src/options.h
            ${PROJECT_SOURCE_DIR}/src/streaming.h
        )

list(APPEND src
            ${PROJECT_SOURCE_DIR}/src/dac_streaming.cpp
            ${PROJECT_SOURCE_DIR}/src/options.cpp
            ${PROJECT_SOURCE_DIR}/src/streaming.cpp
            ${PROJECT_SOURCE_DIR}/src/main.cpp
        )

target_sources(${PROJECT_NAME} PRIVATE ${src})

target_link_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_BINARY_DIR}/bin/
    ${CMAKE_BINARY_DIR}/lib/
    )

target_link_libraries(${PROJECT_NAME} PUBLIC config_net_lib dac_streaming_lib streaming_lib settings_lib)

if(NOT WIN32 )
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread stdc++)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread stdc++ wsock32 ws2_32)
endif()

if(IS_INSTALL)
       install(TARGETS ${PROJECT_NAME}
            DESTINATION ${INSTALL_DIR}/bin)

       install(FILES ${PROJECT_SOURCE_DIR}/streaming-server.run.sh
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
            DESTINATION ${INSTALL_DIR}/bin)
endif()
