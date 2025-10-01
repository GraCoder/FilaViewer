cmake_minimum_required(VERSION 3.26)

set(target_name FilaView)

set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)

find_package(Vulkan)
get_filename_component(VK_BIN_DIR ${Vulkan_GLSLC_EXECUTABLE} DIRECTORY)
get_filename_component(SDL2_LIB_DIR ${Vulkan_LIBRARY} DIRECTORY)
link_directories(${SDL2_LIB_DIR})

find_package(Assimp)

set(filament_dir "" CACHE PATH "")
option(filament_single_thread "" OFF)

####################################################################################
set(gen_root ${CMAKE_CURRENT_BINARY_DIR})
set(resource_dir  "${gen_root}/generated/resources")
set(material_dir  "${gen_root}/generated/material")
set(material_src
  materials/basic.mat
  materials/default.mat
  materials/legacy.mat
  materials/ui_blit.mat

  materials/pc.mat
)
file(MAKE_DIRECTORY ${resource_dir})
file(MAKE_DIRECTORY ${material_dir})
set(resource_bins)
foreach (mat_src ${material_src})
    get_filename_component(localname "${mat_src}" NAME_WE)
    get_filename_component(fullname "${mat_src}" ABSOLUTE)
    set(output_path "${material_dir}/${localname}.filamat")
    add_custom_command(
            OUTPUT ${output_path}
            COMMAND ${filament_dir}/bin/matc -a opengl -a vulkan -o ${output_path} ${fullname}
            DEPENDS ${mat_src}
            COMMENT "Compiling material ${mat_src} to ${output_path}"
    )
    list(APPEND resource_bins ${output_path})
endforeach()
source_group(mat FILES ${resource_bins})
set(resgen_output ${resource_dir}/pcv_mat.c)
add_custom_command(
  OUTPUT ${resgen_output}
  COMMAND ${filament_dir}/bin/resgen -cqx ${resource_dir} -p pcv_mat ${resource_bins}
  DEPENDS ${resource_bins}
  COMMENT "Aggregating resources"
)
####################################################################################

set(env_output)
set(bin2c CACHE FILEPATH "")
set(ibl_tex CACHE FILEPATH "")
set(skybox_tex CACHE FILEPATH "")
if(EXISTS "${ibl_tex}" AND EXISTS "${skybox_tex}")
add_custom_command(OUTPUT ${resource_dir}/ibl.h COMMAND ${bin2c} ${ibl_tex} ${resource_dir}/ibl.h ibl DEPENDS ${ibl_tex})
add_custom_command(OUTPUT ${resource_dir}/skybox.h COMMAND ${bin2c} ${skybox_tex} ${resource_dir}/skybox.h skybox DEPENDS ${skybox_tex})
list(APPEND env_output ${resource_dir}/ibl.h ${resource_dir}/skybox.h)
add_definitions(-DHAS_ENVIRONMENT)
endif()

####################################################################################

set(intern_src
  intern/IWin.cpp
  intern/FTWin.h
  intern/FTWin.cpp
  intern/FTView.h
  intern/FTView.cpp
  intern/FTScene.h
  intern/FTScene.cpp
  intern/ManipOperator.h
  intern/ManipOperator.cpp

  #intern/PCCDispatch.h
  #intern/PCCDispatch.cpp
)

set(imgui_src
  imgui/ImGuiHelper.cpp
  imgui/ImGuiHelper.h
  imgui/ImGUiExtensions.cpp
  imgui/ImGUiExtensions.h
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui_tables.cpp
)

set(mesh_src
  node/mesh/Shape.h
  node/mesh/Shape.cpp
  node/mesh/Cube.h
  node/mesh/Cube.cpp
  node/mesh/Sphere.h
  node/mesh/Sphere.cpp
)

set(node_src
  node/Node.h
  node/Node.cpp
  node/ShapeNode.h
  node/ShapeNode.cpp
  node/GltfNode.h
  node/GltfNode.cpp
  node/Geometry.h
  node/Geometry.cpp
  node/MeshNode.h
  node/MeshNode.cpp
)

if(Assimp_FOUND)
list(APPEND node_src
  node/MeshAssimp.h
  node/MeshAssimp.cpp
)
endif()


set(operator_src
  operator/PickOperator.h
  operator/PickOperator.cpp
)

list(APPEND source_files
  ${intern_src} 
  ${imgui_src} 
  ${mesh_src} 
  ${node_src}
  ${operator_src}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${source_files})

list(APPEND source_files
  IWin.h
  TWin.h 
  TWin.cpp
  TView.h 
  TView.cpp
  TScene.h 
  TScene.cpp
  TOperator.h
  TOperator.cpp
)

add_library(${target_name} SHARED ${source_files} ${resgen_output} ${env_output})

if(filament_single_thread)
target_compile_definitions(${target_name} PRIVATE FILAMENT_SINGLE_THREADED)
endif()

target_compile_definitions(${target_name} PRIVATE NOGDI)

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
  ${filament_dir}/lib/x86_64/filament-iblprefilter.lib
  ${filament_dir}/lib/x86_64/smol-v.lib
  ${filament_dir}/lib/x86_64/utils.lib
  ${filament_dir}/lib/x86_64/camutils.lib
  ${filament_dir}/lib/x86_64/gltfio.lib
  ${filament_dir}/lib/x86_64/gltfio_core.lib
  ${filament_dir}/lib/x86_64/shaders.lib
  ${filament_dir}/lib/x86_64/dracodec.lib
  ${filament_dir}/lib/x86_64/image.lib
  ${filament_dir}/lib/x86_64/ktxreader.lib
  ${filament_dir}/lib/x86_64/basis_transcoder.lib
  ${filament_dir}/lib/x86_64/zstd.lib
)

option(WITH_OPENGL "" OFF)
option(WITH_VULKAN "" ON)

if(WITH_OPENGL)
list(APPEND filament_libs ${filament_dir}/lib/x86_64/bluegl.lib)
target_link_libraries(${target_name} PRIVATE Opengl32)
endif()

if(WITH_VULKAN)
list(APPEND filament_libs ${filament_dir}/lib/x86_64/bluevk.lib)
endif()

target_link_libraries(${target_name} PRIVATE SDL2 ${filament_libs} shlwapi)

if(Assimp_FOUND)
target_link_libraries(${target_name} PRIVATE assimp::assimp)
endif()

#wtf PUBLIC PRIVATE will block the incude directories
target_link_libraries(${target_name} INTERFACE absl::base absl::strings)
#target_link_libraries(${target_name} PRIVATE Opengl32)

