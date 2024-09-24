set(CMAKEVERS 3.22)

cmake_minimum_required(VERSION ${CMAKEVERS})
project(streaming)

if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColourReset "${Esc}[m")
  set(ColourBold  "${Esc}[1m")
  set(Red         "${Esc}[31m")
  set(Green       "${Esc}[32m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Magenta     "${Esc}[35m")
  set(Cyan        "${Esc}[36m")
  set(White       "${Esc}[37m")
  set(BoldRed     "${Esc}[1;31m")
  set(BoldGreen   "${Esc}[1;32m")
  set(BoldYellow  "${Esc}[1;33m")
  set(BoldBlue    "${Esc}[1;34m")
  set(BoldMagenta "${Esc}[1;35m")
  set(BoldCyan    "${Esc}[1;36m")
  set(BoldWhite   "${Esc}[1;37m")
endif()

#set( COMPILER_BIN /usr/bin )
#set( CMAKE_C_COMPILER ${COMPILER_BIN}/gcc CACHE PATH "gcc" FORCE )
#set( CMAKE_CXX_COMPILER ${COMPILER_BIN}/g++ CACHE PATH "g++" FORCE )

option(IS_INSTALL   "Install" ON)
option(RP_PLATFORM  "Redpitaya platform" OFF)
option(PROFILE_ENABLED "Profile enabled" OFF)

option(BUILD_WEB_APP_SERVER "Web app server" ON)
option(BUILD_CONSOLE_SERVER "Console server" ON)
option(BUILD_RPSA_CLIENT "RPSA client" ON)
option(BUILD_PYTHON_LIB "Python lib" OFF)
option(BUILD_RPSA_CLIENT_QT "RPSA client QT" OFF)
option(BUILD_CONVERT_TOOL "Convert tool" ON)


if(NOT DEFINED INSTALL_DIR)
    message(WARNING,"Installation path not set. Installation will be skipped")
    set(IS_INSTALL OFF)
    set(BUILD_WEB_APP_SERVER OFF)
else()
if(EXISTS "${INSTALL_DIR}/include/rp.h" AND EXISTS "${INSTALL_DIR}/rp_sdk")
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
        if (NOT BUILD_RPSA_CLIENT)
            if (NOT BUILD_RPSA_CLIENT_QT)
                set(RP_PLATFORM ON)
            endif()
        endif()
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

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

#add_compile_options(-fsanitize=address)
add_compile_options(-DVERSION=${VERSION} -DREVISION=${REVISION})
add_compile_options(-Wno-psabi $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Debug>:-DTRACE_ENABLE> $<$<CONFIG:Release>:-Os> -ffunction-sections -fdata-sections)

if(NOT WIN32 )
    add_compile_options(-Wall -pedantic -Wextra)
endif()

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=cortex-a9 -mfpu=neon-fp16")
    add_compile_definitions(ARCH_ARM)
endif()

if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

if (RP_PLATFORM)
    add_compile_options(-DRP_PLATFORM)
endif()

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

if (BUILD_PYTHON_LIB)
    add_subdirectory(python_lib)
    add_dependencies(python_lib common_lib)
endif()

if (BUILD_CONVERT_TOOL)
    add_subdirectory(convert_tool)
    add_dependencies(convert_tool common_lib)
endif()

