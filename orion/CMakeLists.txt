cmake_minimum_required(VERSION 3.12)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

add_library(orion STATIC
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
  entity/oentity.h
  entity/camera.h
  entity/camera.cc
  core/asset_registry.h
)
find_package(Vulkan REQUIRED)
find_package(simdjson REQUIRED)

target_include_directories(orion PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/third_party)

target_compile_features(orion PUBLIC cxx_std_20)

target_compile_definitions(orion PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)

target_link_libraries(orion PUBLIC vma glm vkbootstrap stb_image Vulkan::Vulkan SDL2::SDL2 fmt::fmt simdjson::simdjson fastgltf::fastgltf imgui nlohmann)

target_precompile_headers(orion PUBLIC <optional> <vector> <memory> <string> <unordered_map> <glm/mat4x4.hpp> <vulkan/vulkan.h>)
