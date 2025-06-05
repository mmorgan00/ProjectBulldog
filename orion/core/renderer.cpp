#include "renderer.hpp"

#include "renderer_backends/vulkan/vulkan_backend.hpp"

void Renderer::init() {
  // Instantiate the backend
  backend = new VulkanRendererBackend();
  backend->init();
}


void Renderer::shutdown(){
  backend->shutdown();
}

