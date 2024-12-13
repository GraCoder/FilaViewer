cmake_minimum_required(VERSION 3.24)

project(FilaViewer)

include_directories(${CMAKE_SOURCE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/0/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/0/FilaMat/bin/Debug/net9.0-windows8.0)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/0/FilaMat/bin/Release/net9.0-windows8.0)

add_subdirectory(FilaView)
add_subdirectory(FilaMat)
add_subdirectory(ViewTest)
