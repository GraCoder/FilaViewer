cmake_minimum_required(VERSION 3.26.0)

set(CMAKE_CXX_STANDARD 20)

set(target_name ViewForm)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")

set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)

set(source_files
  FilaViewControl.h
  FilaViewControl.cpp
  AssemblyInfo.cpp
)

add_library(${target_name} SHARED ${source_files})
set_target_properties(${target_name} PROPERTIES COMMON_LANGUAGE_RUNTIME "netcore")
set_target_properties(${target_name} PROPERTIES DOTNET_TARGET_FRAMEWORK "net8.0")

target_link_libraries(${target_name} PRIVATE FilaView)
