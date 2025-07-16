cmake_minimum_required(VERSION 3.12)


add_executable(demo 
  main.cc
  entry.h
  util/containers.h
  util/logger.h
  core/renderer.cc
  core/renderer.h
  core/render_engines/vulkan/vulkan_engine.cc
  core/render_engines/vulkan/vulkan_engine.h
  core/render_engines/vulkan/vulkan_initializers.h
  core/render_engines/vulkan/vulkan_initializers.cc
  core/render_engines/vulkan/vulkan_images.cc
  core/render_engines/vulkan/vulkan_images.h
  core/render_engines/vulkan/vulkan_descriptors.h
  core/render_engines/vulkan/vulkan_descriptors.cc
  core/render_engines/vulkan/vulkan_pipelines.h
  core/render_engines/vulkan/vulkan_pipelines.cc
  core/render_engines/vulkan/vulkan_loaders.h
  core/render_engines/vulkan/vulkan_loaders.cc

)

set_property(TARGET demo PROPERTY CXX_STANDARD 20)
target_compile_definitions(demo PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_include_directories(demo PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")


set_target_properties(demo PROPERTIES 
    CXX_STANDARD 20
    COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -DSDL_USE_PIPEWIRE=Off"
    LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS_DEBUG}"
)

target_include_directories(demo PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/third_party 
    ${CMAKE_SOURCE_DIR}/orion/util
    ${CMAKE_SOURCE_DIR}/orion/core
    ${CMAKE_SOURCE_DIR}/orion/core/renderer_backends/vulkan)



target_link_libraries(demo OE_DEMO vma glm vkbootstrap stb_image Vulkan::Vulkan SDL2::SDL2 fmt::fmt simdjson::simdjson fastgltf::fastgltf imgui nlohmann)


target_precompile_headers(demo PUBLIC <optional> <vector> <memory> <string> <vector> <unordered_map> <glm/mat4x4.hpp>  <glm/vec4.hpp> <vulkan/vulkan.h>)
