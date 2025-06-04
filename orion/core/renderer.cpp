#include "renderer.hpp"
#include "renderer_backends/vulkan/vulkan_backend.hpp"


void Renderer::init() {
  // Instantiate the backend
  VulkanRendererBackend rb;
  rb.init();
}





void Renderer::shutdown(){}

