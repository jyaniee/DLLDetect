cmake_minimum_required(VERSION 3.16)

project(DLLDetect VERSION 0.1 LANGUAGES CXX)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg Network)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(SOURCES
    main.cpp
    gui/mainwindow.cpp
    src/ProcessManager.cpp
    resources/resources.qrc
    src/Result.cpp
    src/ProcessManager.cpp
    src/DLLAnalyzer.cpp
)

set(HEADERS
    include/ProcessManager.h
    gui/mainwindow.h
    include/Result.h
    include/DLLAnalyzer.h
)

set(UI_FILES
    gui/mainwindow.ui
)

set(RESOURCE_FILES
    resources/resources.qrc
)

qt6_add_resources(resources_generated resources/resources.qrc)

if(QT_VERSION_MAJOR EQUAL 6)
  qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
  qt6_wrap_ui(UIC_SOURCES ${UI_FILES})
endif()

add_executable(DLLDetect ${SOURCES} ${MOC_SOURCES} ${UIC_SOURCES} ${resources_generated})

target_link_libraries(DLLDetect PRIVATE Qt6::Widgets Qt6::Svg Qt6::Core Qt6::Gui Qt6::Network psapi kernel32)
target_include_directories(DLLDetect PRIVATE include)

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
