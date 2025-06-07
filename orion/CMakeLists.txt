add_executable(demo 
  main.cpp 
  entry.hpp
  core/renderer.cpp
  core/renderer.hpp
  core/renderer_backend.hpp
  core/renderer_backends/vulkan/vulkan_backend.hpp
  core/renderer_backends/vulkan/vulkan_backend.cpp
  core/renderer_backends/vulkan/vulkan_initializers.hpp
  core/renderer_backends/vulkan/vulkan_initializers.cpp
  core/renderer_backends/vulkan/vulkan_descriptors.hpp
  core/renderer_backends/vulkan/vulkan_descriptors.cpp
  core/renderer_backends/vulkan/vulkan_pipelines.hpp
  core/renderer_backends/vulkan/vulkan_pipelines.cpp
  core/renderer_backends/vulkan/vulkan_images.cpp
  core/renderer_backends/vulkan/vulkan_images.hpp
  util/containers.hpp
  util/logger.hpp
)

set_target_properties(demo PROPERTIES 
    CXX_STANDARD 20
    COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}"
    LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS_DEBUG}"
)

target_include_directories(demo PUBLIC
    ${CMAKE_SOURCE_DIR}/util
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/core/renderer_backends/vulkan)

target_link_libraries(demo OE_DEMO vma glm vkbootstrap stb_image Vulkan::Vulkan SDL2::SDL2 fmt::fmt simdjson::simdjson fastgltf::fastgltf)
