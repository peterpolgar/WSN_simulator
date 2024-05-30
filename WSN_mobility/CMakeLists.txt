cmake_minimum_required(VERSION 3.5)

project(WSN_mobility VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-O2")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Concurrent)

set(PROJECT_SOURCES
        main.cpp
        widget.cpp
        widget.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(WSN_mobility
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        cevent_space.h cevent_space.cpp
        cslider.h cslider.cpp
        ikonok.qrc
        csim_run.h csim_run.cpp
        mobility_interface.h
        placement_interface.h
        cfp_slider.h cfp_slider.cpp
        cstacked_widget.h cstacked_widget.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET WSN_mobility APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(WSN_mobility SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(WSN_mobility
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(WSN_mobility PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(WSN_mobility PRIVATE Qt6::Concurrent)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.WSN_mobility)
endif()
set_target_properties(WSN_mobility PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS WSN_mobility
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(WSN_mobility)
endif()

add_subdirectory(plugins)
