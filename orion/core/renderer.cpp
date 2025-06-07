#include "renderer.hpp"

#include "renderer_backends/vulkan/vulkan_backend.hpp"
#include "util/logger.hpp"

DECLARE_LOG_CATEGORY(RENDERER);

void Renderer::init(app_state& state) {
  if(state.graphicsAPI == "Vulkan"){
  // Instantiate the backend
  OE_LOG(RENDERER, INFO, "Initializing Vulkan renderer");
  backend = new VulkanRendererBackend();
  backend->init(state);

  }
  else{
  OE_LOG(RENDERER, FATAL, "Unsupported graphics API {} specified", state.graphicsAPI);
  }
}

void Renderer::run(){
// TODO: Platform driven resize
  backend->run();
}

void Renderer::shutdown(){
  backend->shutdown();
}

