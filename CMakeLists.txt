cmake_minimum_required(VERSION 3.16)
project(VideoPlayerApp VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia MultimediaWidgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ui)

include_directories(include)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/videoplayer.cpp
    src/subtitle.cpp
    src/infobox.cpp
)

set(HEADERS
    include/mainwindow.h
    include/videoplayer.h
    include/subtitle.h
    include/infobox.h
)

set(FORMS
    ui/mainwindow.ui
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${FORMS})

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)

# 라즈베리파이 최적화
if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
    target_link_libraries(${PROJECT_NAME} ${GSTREAMER_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${GSTREAMER_INCLUDE_DIRS})
endif()