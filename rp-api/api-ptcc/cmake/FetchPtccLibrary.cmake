# Clones the upstream VIGO ptcc-library so its Python package can be installed
# next to the bridge module. The C++ library is only a wrapper: this package is
# what actually implements the protocol at runtime.
#
# With PTCC_FETCH_UPSTREAM=OFF, or when git is unavailable, the clone is
# skipped and the package is expected to be already present in
# ${INSTALL_DIR}/lib/python or on the system PYTHONPATH.

set(PTCC_UPSTREAM_DIR ${CMAKE_BINARY_DIR}/_upstream/ptcc-library)
set(PTCC_UPSTREAM_PACKAGE "" CACHE INTERNAL "Path to the upstream ptcc_library package")

if(NOT PTCC_FETCH_UPSTREAM)
    message(STATUS "PTCC upstream: fetch disabled, expecting ptcc_library on PYTHONPATH")
    return()
endif()

find_package(Git QUIET)
if(NOT Git_FOUND)
    message(WARNING "PTCC upstream: git not found, expecting ptcc_library on PYTHONPATH")
    return()
endif()

message(STATUS "PTCC upstream: ${PTCC_UPSTREAM_URL} @ ${PTCC_UPSTREAM_TAG}")

if(EXISTS ${PTCC_UPSTREAM_DIR}/.git)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} fetch --depth 1 origin ${PTCC_UPSTREAM_TAG}
        WORKING_DIRECTORY ${PTCC_UPSTREAM_DIR}
        RESULT_VARIABLE PTCC_GIT_RESULT
        OUTPUT_QUIET ERROR_QUIET
    )
    if(PTCC_GIT_RESULT EQUAL 0)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} checkout --detach FETCH_HEAD
            WORKING_DIRECTORY ${PTCC_UPSTREAM_DIR}
            RESULT_VARIABLE PTCC_GIT_RESULT
            OUTPUT_QUIET ERROR_QUIET
        )
    endif()
else()
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/_upstream)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} clone --depth 1 --branch ${PTCC_UPSTREAM_TAG}
                ${PTCC_UPSTREAM_URL} ${PTCC_UPSTREAM_DIR}
        RESULT_VARIABLE PTCC_GIT_RESULT
        OUTPUT_QUIET ERROR_QUIET
    )
endif()

if(NOT PTCC_GIT_RESULT EQUAL 0 OR NOT EXISTS ${PTCC_UPSTREAM_DIR}/ptcc_library)
    message(WARNING "PTCC upstream: clone failed (${PTCC_GIT_RESULT}), "
                    "expecting ptcc_library on PYTHONPATH")
    return()
endif()

set(PTCC_UPSTREAM_PACKAGE ${PTCC_UPSTREAM_DIR}/ptcc_library CACHE INTERNAL "" FORCE)
message(STATUS "PTCC upstream: package at ${PTCC_UPSTREAM_PACKAGE}")
