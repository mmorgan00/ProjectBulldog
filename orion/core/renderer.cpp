#include "renderer.hpp"

#include "renderer_backends/vulkan/vulkan_backend.hpp"
#include "util/logger.hpp"

DECLARE_LOG_CATEGORY(RENDERER);

void Renderer::init(app_state& state) {
  if(state.graphicsAPI == "Vulkan"){
  // Instantiate the backend
  backend = new VulkanRendererBackend();
  backend->init(state);

  }
  else{
  OE_LOG(RENDERER, FATAL, "Unsupported graphics API {} specified", state.graphicsAPI);
  }
}


void Renderer::shutdown(){
  backend->shutdown();
}

