cmake_minimum_required(VERSION ${CMAKEVERS})

project(board_lib)

## The board for the hardware facade of uio_lib: the profile, the calibration and the front end
## relays through the Red Pitaya libraries on a board, a dummy on the host.

add_library(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/board.h)

target_include_directories(${PROJECT_NAME}
    PUBLIC  ${PROJECT_SOURCE_DIR}
            ${COMMON_LIB_DIR}
            ${CMAKE_SOURCE_DIR}
            )

target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/board.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC uio_lib logger_lib)

if (RP_PLATFORM)
    target_include_directories(${PROJECT_NAME} PRIVATE ${INSTALL_DIR}/include)
    target_link_directories(${PROJECT_NAME} PUBLIC ${INSTALL_DIR}/lib/)
    target_link_libraries(${PROJECT_NAME} PUBLIC rp-hw rp-hw-calib rp-hw-profiles rp-i2c i2c)
endif()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PROJECT_SOURCE_DIR}/board.h
        "${CMAKE_BINARY_DIR}/bin/include/${PROJECT_NAME}"
    COMMENT "Copying ${PROJECT_NAME} public headers to ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME}"
    VERBATIM)
