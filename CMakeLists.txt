cmake_minimum_required(VERSION 3.26)

project(FilaViewer)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/0/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/0/CSViewer/bin/Debug/net9.0-windows8.0)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/0/CSViewer/bin/Release/net9.0-windows8.0)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/Third)

set(vc_lib "" CACHE PATH "")
if(IS_DIRECTORY ${vc_lib})
include_directories(${vc_lib}/include)
endif()

find_package(absl CONFIG REQUIRED)

add_subdirectory(ViewTest)
add_subdirectory(FilaView)
add_subdirectory(CCViewer)
