cmake_minimum_required(VERSION ${CMAKEVERS})
project(web_server)

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

message(STATUS "${Yellow}Project=${PROJECT_NAME}${ColourReset}")
message(STATUS "RedPitaya platform=${RP_PLATFORM}")
message(STATUS "Is install ${IS_INSTALL}")
message(STATUS "Install path ${INSTALL_DIR}")
message(STATUS "VERSION=${VERSION}")
message(STATUS "REVISION=${REVISION}")
message(STATUS "BUILD_NUMBER=${BUILD_NUMBER}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "LINUX_VER=${LINUX_VER}")

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

## The backend of the streaming web page (rp_sdk parameters, the session). streaming-server links
## it and runs it as redpitaya_streaming.service; it is no bazaar module (controllerhf.so) any more.

add_compile_options($<$<CONFIG:Debug>:-DZIP_DISABLED>)

list(APPEND headers
            ${PROJECT_SOURCE_DIR}/src/main.h
        )

list(APPEND src
            ${PROJECT_SOURCE_DIR}/src/main.cpp
        )

add_library(web_backend STATIC ${src} ${headers})
target_compile_options(web_backend PRIVATE -DRP_PLATFORM -Wno-reorder -Wno-pedantic -Wno-unused-parameter)
target_include_directories(web_backend
    PUBLIC  ${PROJECT_SOURCE_DIR}
            ${CMAKE_BINARY_DIR}/bin/include
            ${INSTALL_DIR}/include ${INSTALL_DIR}/rp_sdk ${INSTALL_DIR}/rp_sdk/libjson)
target_link_directories(web_backend PUBLIC ${INSTALL_DIR}/lib/ ${INSTALL_DIR}/lib/web ${INSTALL_DIR}/rp_sdk ${CMAKE_BINARY_DIR}/bin/ ${CMAKE_BINARY_DIR}/lib/)
target_link_libraries(web_backend PUBLIC session_lib settings_lib board_lib rp_sdk rp rp-hw rp-hw-calib rp-hw-profiles rp-client rp-i2c rp-spi rp-gpio i2c z pthread stdc++)
