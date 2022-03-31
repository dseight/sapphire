cmake_minimum_required(VERSION 3.11)

project(harbour-sapphire CXX)

set(CMAKE_AUTOMOC ON)

find_package(PkgConfig REQUIRED)
include(GNUInstallDirs)
include(CMakeFindDependencyMacro)

find_dependency(Qt5 COMPONENTS Quick Gui WebSockets REQUIRED)
pkg_check_modules(SAILFISHAPP REQUIRED sailfishapp)
pkg_check_modules(MLITE5 REQUIRED mlite5)

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(qmdnsengine EXCLUDE_FROM_ALL)

add_executable(${CMAKE_PROJECT_NAME}
    src/deviceinfo.cpp
    src/sketchartboard.cpp
    src/sketchdocument.cpp
    src/sketchpage.cpp
    src/sketchserver.cpp
    src/sketchservermodel.cpp
    src/qqmlobjectlistmodel.h
    src/main.cpp)

set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 14)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    VERSION_STRING=\"${VERSION}\")

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${SAILFISHAPP_INCLUDE_DIRS}
    ${MLITE5_INCLUDE_DIRS})

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    Qt5::Gui
    Qt5::Quick
    Qt5::WebSockets
    qmdnsengine
    ${SAILFISHAPP_LDFLAGS}
    ${MLITE5_LDFLAGS})

install(TARGETS ${CMAKE_PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES ${CMAKE_PROJECT_NAME}.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

install(DIRECTORY qml/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/qml
        FILES_MATCHING
        PATTERN "*.qml"
        PATTERN "*.svg")

foreach(size 86x86 108x108 128x128 172x172)
    install(FILES icons/${size}/${CMAKE_PROJECT_NAME}.png
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${size}/apps)
endforeach()
