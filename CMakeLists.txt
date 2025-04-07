cmake_minimum_required(VERSION 3.16)

project(DLLDetect VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

include_directories(include)
qt_add_resources(RESOURCES resources.qrc)
# qt_wrap_ui(UISrcs gui/mainwindow.ui)
add_subdirectory(src)
set(PROJECT_SOURCES
    main.cpp
    gui/mainwindow.cpp
    gui/mainwindow.ui
    gui/mainwindow.h
)
qt_add_executable(DLLDetect
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    ${RESOURCES}
)

target_link_libraries(DLLDetect PRIVATE Qt${QT_VERSION_MAJOR}::Widgets core)
target_include_directories(DLLDetect PRIVATE include)
target_sources(DLLDetect PRIVATE ${RESOURCES})
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.DLLDetect)
endif()

set_target_properties(DLLDetect PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS DLLDetect
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(DLLDetect)
endif()
