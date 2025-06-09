cmake_minimum_required(VERSION 3.26)

set(target_name CCViewer)

set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)

find_package(Qt4 REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(src
		main.cpp

		FilaView.h
		FilaView.cpp

		MainWindow.h
		MainWindow.cpp
		WindowBar.h
		WindowBar.cpp
)

add_executable(${target_name} ${src} ${hdr} ${mocfiles})

target_include_directories(${target_name} PRIVATE ${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR})

target_include_directories(${target_name} PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(${target_name} Imm32 dwrite ddraw)
target_link_libraries(${target_name} ${QT_QTCORE_LIBRARY_RELEASE} ${QT_QTGUI_LIBRARY_RELEASE})

target_link_libraries(${target_name} FilaView)
