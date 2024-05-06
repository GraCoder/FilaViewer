cmake_minimum_required(VERSION 3.24)

project(FilaViewer)

include_directories(${CMAKE_SOURCE_DIR})

set(output_dir "" CACHE PATH "")

if(DEFINED output_dir)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${output_dir})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${output_dir})
else()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

add_subdirectory(FilaView)
add_subdirectory(FilaMat)
add_subdirectory(ViewTest)
