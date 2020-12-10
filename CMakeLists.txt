cmake_minimum_required(VERSION 3.5)

project(magnifiqus LANGUAGES CXX)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

set (CMAKE_AUTOUIC ON)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets X11Extras REQUIRED)

set (RESOURCE_FILES resources/resources.qrc)
set (SOURCE_FILES
    src/mainwindow.h
    src/mainwindow.cpp
    src/main.cpp
)
add_executable(${PROJECT_NAME}
    ${SOURCE_FILES}
    ${RESOURCE_FILES}
)
source_group ("Resource Files" FILES ${RESOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE "src")
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::X11Extras X11)

if (UNIX AND NOT APPLE)
    include (GNUInstallDirs)
    install (TARGETS ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_BINDIR})
    install (FILES resources/icons/${PROJECT_NAME}.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
    install (FILES resources/linux/${PROJECT_NAME}.appdata.xml
        DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
    install (FILES resources/linux/${PROJECT_NAME}.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
#   install (FILES ${QM_FILES}
#       DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/translations)
endif()

