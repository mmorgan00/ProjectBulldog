// Copyright 2025 Max Morgan

#include "core/renderer.h"

#include "core/render_engines/vulkan/vulkan_engine.h"
#include "util/logger.h"

DECLARE_LOG_CATEGORY(RENDERER);

RenderEngine *engine;

VulkanEngine vkEngine;
void Renderer::init(app_state &state) {
  if (state.graphicsAPI == "Vulkan") {
    // Instantiate the backend
    OE_LOG(RENDERER, INFO, "Initializing Vulkan renderer");
    vkEngine.init(state);
    engine = &vkEngine;
  } else {
    OE_LOG(RENDERER, FATAL, "Unsupported graphics API {} specified",
           state.graphicsAPI);
  }
}

void Renderer::draw() {
  // TODO: Platform driven resize
  engine->draw();
}

void Renderer::cleanup() {
  engine->cleanup();
  engine = nullptr;
}
