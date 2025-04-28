cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 20)

set(target_name ViewTest)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")

set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)

set(source_files
  main.cpp
)

add_executable(${target_name} ${source_files})

target_link_libraries(${target_name} PRIVATE FilaView)