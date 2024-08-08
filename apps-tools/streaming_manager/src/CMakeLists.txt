cmake_minimum_required(VERSION 3.18)
project(streaming)


#set( COMPILER_BIN /usr/bin )
#set( CMAKE_C_COMPILER ${COMPILER_BIN}/gcc CACHE PATH "gcc" FORCE )
#set( CMAKE_CXX_COMPILER ${COMPILER_BIN}/g++ CACHE PATH "g++" FORCE )

option(IS_INSTALL   "Install" ON)
option(RP_PLATFORM  "Redpitaya platform" OFF)

option(BUILD_WEB_APP_SERVER "Web app server" ON)
option(BUILD_CONSOLE_SERVER "Console server" ON)
option(BUILD_RPSA_CLIENT "RPSA client" ON)
option(BUILD_RPSA_CLIENT_QT "RPSA client QT" OFF)
option(BUILD_CONVERT_TOOL "Convert tool" ON)

if(NOT DEFINED INSTALL_DIR)
    message(WARNING,"Installation path not set. Installation will be skipped")
    set(IS_INSTALL OFF)
    set(BUILD_WEB_APP_SERVER OFF)
else()
if(EXISTS "${INSTALL_DIR}/include/rp.h" AND EXISTS "${INSTALL_DIR}/rp_sdk")
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
        set(RP_PLATFORM ON)
    endif()
    message(STATUS,"Redpitaya platform mode set in ${RP_PLATFORM}")

else()
    set(RP_PLATFORM OFF)
    message(STATUS,"Redpitaya platform mode set in ${RP_PLATFORM}" )
endif()
endif()


if(NOT DEFINED VERSION)
  set(VERSION "0")
endif()

if(NOT DEFINED REVISION)
  set(REVISION "DEV")
endif()

if(NOT DEFINED BUILD_NUMBER)
  set(BUILD_NUMBER "dev")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install_dir")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
#set(CMAKE_CXX_FLAGS -fsanitize=address)


add_subdirectory(common_lib)

if (BUILD_CONSOLE_SERVER)
    add_subdirectory(streaming-server)
    add_dependencies(streaming-server common_lib)
endif()

if (BUILD_WEB_APP_SERVER AND RP_PLATFORM)
    add_subdirectory(web_server)
    add_dependencies(web_server common_lib)
endif()

if (BUILD_RPSA_CLIENT)
    add_subdirectory(rpsa_client)
    add_dependencies(rpsa_client common_lib)
endif()

if (BUILD_RPSA_CLIENT_QT)
    add_subdirectory(rpsa_client_qt)
    add_dependencies(rpsa_client_qt common_lib)
endif()

if (BUILD_CONVERT_TOOL)
    add_subdirectory(convert_tool)
    add_dependencies(convert_tool common_lib)
endif()
