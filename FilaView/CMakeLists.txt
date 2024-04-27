cmake_minimum_required(VERSION 3.26.0)

set(CMAKE_CXX_STANDARD 20)

set(target_name FilaView)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")

set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)

find_package(Vulkan)
get_filename_component(VK_BIN_DIR ${Vulkan_GLSLC_EXECUTABLE} DIRECTORY)
get_filename_component(SDL2_LIB_DIR ${Vulkan_LIBRARY} DIRECTORY)
link_directories(${SDL2_LIB_DIR})

set(filament_dir "" CACHE PATH "")

set(gen_root ${CMAKE_CURRENT_BINARY_DIR})
set(resource_dir  "${gen_root}/generated/resources")
set(material_dir  "${gen_root}/generated/material")

set(intern_src
  intern/FTWin.h
  intern/FTWin.cpp
  intern/FTView.h
  intern/FTView.cpp
  intern/FTScene.h
  intern/FTScene.cpp

  #intern/PCCDispatch.h
  #intern/PCCDispatch.cpp
)
source_group(intern FILES ${intern_src})

#set(imgui_src
#  imgui/ImGuiHelper.cpp
#  imgui/ImGuiHelper.h
#  imgui/ImGUiExtensions.cpp
#  imgui/ImGUiExtensions.h
#  imgui/imgui.cpp
#  imgui/imgui_draw.cpp
#  imgui/imgui_widgets.cpp
#  imgui/imgui_tables.cpp
#)
#source_group(imgui FILES ${imgui_src})

set(source_files
  FilaView.h
  FilaView.cpp
  AssemblyInfo.cpp

  TView.h
  TView.cpp
  TWin.h
  TWin.cpp
  TScene.h
  TScene.cpp
)

add_library(${target_name} SHARED ${source_files} ${intern_src} ${imgui_src} ${source_mesh} ${resgen_output})
set_target_properties(${target_name} PROPERTIES COMMON_LANGUAGE_RUNTIME "netcore")
set_target_properties(${target_name} PROPERTIES DOTNET_TARGET_FRAMEWORK "net8.0")

target_compile_definitions(${target_name} PRIVATE NOGDI)
target_compile_definitions(${target_name} PRIVATE PC_DEBUG)

target_include_directories(${target_name} PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(${target_name} PRIVATE ${filament_dir}/include)
target_include_directories(${target_name} PRIVATE ${resource_dir})

target_include_directories(${target_name} PRIVATE ${Vulkan_INCLUDE_DIR})

set(filament_libs
  ${filament_dir}/lib/x86_64/filament.lib
  ${filament_dir}/lib/x86_64/filabridge.lib
  ${filament_dir}/lib/x86_64/filamat.lib
  ${filament_dir}/lib/x86_64/filaflat.lib
  ${filament_dir}/lib/x86_64/viewer.lib
  ${filament_dir}/lib/x86_64/backend.lib
  ${filament_dir}/lib/x86_64/geometry.lib
  ${filament_dir}/lib/x86_64/ibl.lib
  ${filament_dir}/lib/x86_64/smol-v.lib
  ${filament_dir}/lib/x86_64/utils.lib
  ${filament_dir}/lib/x86_64/camutils.lib
  ${filament_dir}/lib/x86_64/bluevk.lib
  ${filament_dir}/lib/x86_64/gltfio.lib
  ${filament_dir}/lib/x86_64/gltfio_core.lib
  ${filament_dir}/lib/x86_64/dracodec.lib
)

target_link_libraries(${target_name} PRIVATE SDL2 ${filament_libs})
