cmake_minimum_required(VERSION 3.19)

project(rpsa_client_qt VERSION 0.1 LANGUAGES CXX)

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

message(STATUS "Compiler С path: ${CMAKE_C_COMPILER}")
message(STATUS "Compiler С ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "Compiler С version: ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "Compiler С is part: ${CMAKE_COMPILER_IS_GNUC}")

message(STATUS "Compiler С++ path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler С++ ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler С++version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Compiler С++ is part: ${CMAKE_COMPILER_IS_GNUCXX}")
message(STATUS "QT: ${QT}")
message(STATUS "QT_DIR: ${QT_DIR}")
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
list( APPEND CMAKE_FIND_ROOT_PATH "${QT}" )
message(STATUS "CMAKE_FIND_ROOT_PATH: ${CMAKE_FIND_ROOT_PATH}" )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick Gui Qml REQUIRED PATHS ${QT_DIR} ${CMAKE_PREFIX_PATH}  NO_DEFAULT_PATH)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick Gui Charts Widgets REQUIRED PATHS ${QT_DIR} ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)

set(CMAKE_AUTOUIC OFF)

set(HEADER_FILES
        src/ui_controller.h
        src/models/boardsmodel.h
        src/models/board.h
        src/models/consolemodel.h
        src/logic/chartdataholder.h
        src/logic/device_logic.h
        )


set(PROJECT_SOURCES
        src/main.cpp
        src/ui_controller.cpp
        src/models/boardsmodel.cpp
        src/models/board.cpp
        src/models/consolemodel.cpp
        src/logic/chartdataholder.cpp
        src/logic/device_logic.cpp
        qml.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(rpsa_client_qt
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET rpsa_client_qt APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(rpsa_client_qt SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(rpsa_client_qt
          ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC  ${PROJECT_SOURCE_DIR}
            ${CMAKE_BINARY_DIR}/bin/include
            )

target_compile_options(${PROJECT_NAME}
            PRIVATE -DVERSION=${VERSION} -DREVISION=${REVISION} $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Release>:-Os> -ffunction-sections -fdata-sections)

target_link_directories(${PROJECT_NAME}
            PRIVATE
            ${CMAKE_BINARY_DIR}/bin/
            ${CMAKE_BINARY_DIR}/lib/
            )

target_sources( ${PROJECT_NAME}
                PRIVATE
                ${PROJECT_SOURCES}
                ${HEADER_FILES})

target_compile_definitions(rpsa_client_qt
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(rpsa_client_qt
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Quick Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Charts Qt${QT_VERSION_MAJOR}::Qml  Qt${QT_VERSION_MAJOR}::Widgets)

target_link_libraries(${PROJECT_NAME} PRIVATE config_net_lib dac_streaming_lib converter_lib streaming_lib settings_lib)

if(NOT WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread stdc++ gcc)
else()
    set(WIN_SUFFIX -win)
    target_link_libraries(${PROJECT_NAME} PRIVATE -static pthread wsock32 ws2_32 gcc stdc++)
endif()


set_target_properties(rpsa_client_qt PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER rpsa_steraming_qt
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_qml_plugins(rpsa_client_qt)
    qt_finalize_executable(rpsa_client_qt)
endif()

if(IS_INSTALL)
      install(TARGETS ${PROJECT_NAME}
              DESTINATION ${INSTALL_DIR}/streaming/out/rpsa_client-qt_${VERSION}-${REVISION}${WIN_SUFFIX})

if(WIN32)
	set(EXE_EXT ".exe")
endif()
    install(CODE "execute_process(COMMAND /usr/bin/cqtdeployer -bin ${INSTALL_DIR}/streaming/out/rpsa_client-qt_${VERSION}-${REVISION}${WIN_SUFFIX}/rpsa_client_qt${EXE_EXT} -qmake ${QT}/bin/qmake -qmlDir ${QT}/qml -targetDir ${INSTALL_DIR}/streaming/rpsa_client-qt_${VERSION}-${REVISION}${WIN_SUFFIX})")
if(WIN32)
#    install(CODE "execute_process(COMMAND cp /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll ${INSTALL_DIR}/streaming/rpsa_client-qt_${VERSION}-${REVISION}${WIN_SUFFIX}/lib/libwinpthread-1.dll)")
#    install(CODE "execute_process(COMMAND cp -f /usr/lib/gcc/x86_64-w64-mingw32/9.3-posix/*.dll ${INSTALL_DIR}/streaming/rpsa_client-qt_${VERSION}-${REVISION}${WIN_SUFFIX}/lib/)")
    install(FILES /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll DESTINATION ${INSTALL_DIR}/streaming/rpsa_client-qt_${VERSION}-${REVISION}${WIN_SUFFIX}/lib/)
    install(DIRECTORY /usr/lib/gcc/x86_64-w64-mingw32/9.3-posix/ DESTINATION ${INSTALL_DIR}/streaming/rpsa_client-qt_${VERSION}-${REVISION}${WIN_SUFFIX}/lib/ FILES_MATCHING PATTERN "*.dll")
endif()
    install(CODE "execute_process(COMMAND rm -rf ${INSTALL_DIR}/streaming/out)")
endif()
