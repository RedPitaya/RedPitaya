cmake_minimum_required(VERSION ${CMAKEVERS})

project(settings_lib)

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)

message(STATUS "${Yellow}Project=${PROJECT_NAME}${ColourReset}")
message(STATUS "RedPitaya platform=${RP_PLATFORM}")
message(STATUS "CPU ARCH=${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "VERSION=${VERSION}")
message(STATUS "REVISION=${REVISION}")
message(STATUS "BUILD_NUMBER=${BUILD_NUMBER}")
message(STATUS "LINUX_VER=${LINUX_VER}")

message(STATUS "Compiler С path: ${CMAKE_C_COMPILER}")
message(STATUS "Compiler С ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "Compiler С version: ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "Compiler С is part: ${CMAKE_COMPILER_IS_GNUC}")

message(STATUS "Compiler С++ path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler С++ ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler С++version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Compiler С++ is part: ${CMAKE_COMPILER_IS_GNUCXX}")

include(FetchContent)

set (BUILD_SHARED_LIBS OFF CACHE INTERNAL "Turn off shared")
set (JSONCPP_WITH_TESTS OFF CACHE INTERNAL "Turn off jsontest")
set (JSONCPP_WITH_POST_BUILD_UNITTEST OFF CACHE INTERNAL "Turn off JSONCPP_WITH_POST_BUILD_UNITTEST")
set (JSONCPP_WITH_PKGCONFIG_SUPPORT OFF CACHE INTERNAL "Turn off JSONCPP_WITH_PKGCONFIG_SUPPORT")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-deprecated")

# if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
#     set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=cortex-a9 -mfpu=neon-fp16 -Wno-deprecated-declarations -Wno-deprecated")
# endif()

# if(NOT WIN32)
#     set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
# endif()

FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp
        GIT_TAG 1.9.4
        )
FetchContent_MakeAvailable(json)

FILE(GLOB_RECURSE INC_ALL "*.h")

add_library(${PROJECT_NAME} ${INC_ALL})

target_include_directories(${PROJECT_NAME}
    PUBLIC  ${PROJECT_SOURCE_DIR}
            ${COMMON_LIB_DIR}
            ${json_SOURCE_DIR}/include)

list(APPEND headers
            ${PROJECT_SOURCE_DIR}/enums.h
            ${PROJECT_SOURCE_DIR}/dac_settings_client.h
            ${PROJECT_SOURCE_DIR}/stream_settings.h
        )

    list(APPEND src
            ${PROJECT_SOURCE_DIR}/stream_settings.cpp
        )

target_sources(${PROJECT_NAME} PRIVATE ${src})

#add_library(jsoncpp STATIC IMPORTED json)
#set_property(TARGET jsoncpp PROPERTY
#        IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/bin/libjsoncpp.a)

target_link_libraries(${PROJECT_NAME}
    PUBLIC pthread stdc++ jsoncpp)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${headers}
        "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}"
    COMMENT "Copying ${PROJECT_NAME} public headers to ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME}"
    VERBATIM)
