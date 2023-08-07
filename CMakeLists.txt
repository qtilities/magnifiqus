cmake_minimum_required(VERSION 3.15)
project(Magnifiqus
    VERSION 0.3.0
    LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#===============================================================================
# Qt
#===============================================================================
option(PROJECT_TRANSLATIONS_UPDATE "Update source translations [default: OFF]" OFF)
set(PROJECT_QT_VERSION 5 CACHE STRING "Qt version to use [default: 5]")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(QT NAMES Qt${PROJECT_QT_VERSION})
list(APPEND components_ "LinguistTools;Widgets")
if(QT_VERSION_MAJOR EQUAL 5)
    list(APPEND components_ "X11Extras")
endif()
find_package(Qt${QT_VERSION_MAJOR} REQUIRED ${components_})
find_package(Qtilitools REQUIRED)

find_package(X11 REQUIRED)
#===============================================================================
# Project files
#===============================================================================
set(PROJECT_SOURCES
    src/application.hpp
    src/application.cpp
    src/litebutton.hpp
    src/litebutton.cpp
    src/dialogabout.hpp
    src/dialogabout.cpp
    src/dialogprefs.hpp
    src/dialogprefs.cpp
    src/mainwindow.hpp
    src/mainwindow.cpp
    src/qtilities.hpp
    src/settings.hpp
    src/settings.cpp
    src/systemtrayicon.hpp
    src/systemtrayicon.cpp
    src/x11.hpp
    src/x11.cpp
)
set(PROJECT_UI_FILES
    src/dialogabout.ui
    src/dialogprefs.ui
)
set(PROJECT_OTHER_FILES
    .github/workflows/build.yml
    .clang-format
    .editorconfig
    .gitattributes
    .gitignore
    README.md
)
source_group("Other Files" FILES ${PROJECT_OTHER_FILES})
source_group("UI Files" FILES ${PROJECT_UI_FILES})
#===============================================================================
# Resources
#===============================================================================
include(Config.cmake)
include(QtAppResources)
#===============================================================================
# Application executable
#===============================================================================
set(PROJECT_ALL_FILES
    ${PROJECT_DESKTOP_FILES}
    ${PROJECT_RESOURCES}
    ${PROJECT_SOURCES}
    ${PROJECT_OTHER_FILES}
    ${PROJECT_QM_FILES}
    ${PROJECT_TRANSLATION_SOURCES}
    ${PROJECT_UI_FILES}
)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    # Define target properties for Android with Qt 6 as:
    #    set_property(TARGET test APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    #                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
    # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
    qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION ${PROJECT_ALL_FILES})
else()
    if(ANDROID)
        # Define properties for Android with Qt 5 after find_package() calls as:
        #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
        add_library(${PROJECT_NAME} SHARED ${PROJECT_ALL_FILES})
    else()
        add_executable(${PROJECT_NAME} ${PROJECT_ALL_FILES})
    endif()
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${X11_INCLUDE_DIR}
    "src"
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${X11_LIBRARIES}
    Qt::Widgets
)
if(QT_VERSION_MAJOR EQUAL 5)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt::X11Extras)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_ID}")

target_compile_definitions(${PROJECT_NAME} PRIVATE
    APPLICATION_DISPLAY_NAME="${PROJECT_NAME}"
    APPLICATION_NAME="${PROJECT_ID}"
    ORGANIZATION_DOMAIN="${PROJECT_ORGANIZATION_URL}"
    ORGANIZATION_NAME="${PROJECT_ORGANIZATION_NAME}"
    PROJECT_DATA_DIR="${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_ID}"
    PROJECT_ICON_NAME="${PROJECT_ICON_FILE_NAME}"
    PROJECT_ICON_SYSTEM_PATH="${PROJECT_ICON_FILE_PATH}/${PROJECT_ICON_FILE_NAME}"
)
#===============================================================================
# Install application
#===============================================================================
if (UNIX AND NOT APPLE)
    install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER ${PROJECT_ORGANIZATION_URL}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
    )
    install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
#===============================================================================
# Project information
#===============================================================================
message(STATUS "
Project name:                     ${PROJECT_NAME}
AppStream ID:                     ${PROJECT_APPSTREAM_ID}
Version:                          ${PROJECT_VERSION}
Qt version:                       ${QT_VERSION}
Build type:                       ${CMAKE_BUILD_TYPE}
Install prefix:                   ${CMAKE_INSTALL_PREFIX}
Update translations before build: ${PROJECT_TRANSLATIONS_UPDATE}

CXX Debug flags:                  ${CMAKE_CXX_FLAGS_DEBUG}
CXX Release flags:                ${CMAKE_CXX_FLAGS_RELEASE}
CXX MinSize flags:                ${CMAKE_CXX_FLAGS_MINSIZEREL}
CXX RelWithDebInfo flags:         ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}
")

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()
