cmake_minimum_required(VERSION 3.24)

project(FilaViewer)

include_directories(${CMAKE_SOURCE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(FilaView)
add_subdirectory(ViewForm)
add_subdirectory(FilaMat)
add_subdirectory(ViewTest)
