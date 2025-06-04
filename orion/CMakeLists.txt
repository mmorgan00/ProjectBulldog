add_executable(demo 
  main.cpp 
  entry.hpp
  core/renderer.cpp
  core/renderer.hpp
  core/renderer_backend.hpp
  core/renderer_backends/vulkan/vulkan_backend.hpp
  core/renderer_backends/vulkan/vulkan_backend.cpp
)

set_target_properties(demo PROPERTIES 
    CXX_STANDARD 20
    COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}"
    LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS_DEBUG}"
)

target_link_libraries(demo OE_DEMO vma vkbootstrap Vulkan::Vulkan SDL2::SDL2)
