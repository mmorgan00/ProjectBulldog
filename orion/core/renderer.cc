// Copyright 2025 Max Morgan

#include "core/renderer.h"

#include <entity/camera.h>

#include <memory>
#include <string>

#include "core/render_engines/vulkan/vulkan_engine.h"
#include "util/logger.h"

void Renderer::init(app_state& state) {
  if (state.graphicsAPI == "Vulkan") {
    engine = std::make_unique<VulkanEngine>();
    engine->init(state);
    // Instantiate the backend
    OE_LOG(RENDERER, INFO, "Initializing Vulkan renderer");
  } else {
    OE_LOG(RENDERER, FATAL, "Unsupported graphics API {} specified",
           state.graphicsAPI);
  }
}

void Renderer::resize_window() { engine->resize_window(); }

/**
 * @brief Driver function to render each frame
 */
void Renderer::draw() { engine->draw(); }

/**
 * brief Sets a designated camera to be used as the Renderer view matrix
 */
void Renderer::set_camera(Camera* camera) { engine->set_camera(camera); }
/**
 * @detail loads a single object from file to add to the scene graph
 */
std::shared_ptr<RenderComponent> Renderer::loadScene(
    std::string_view fileName) {
  // TODO: Properly implement
  this->engine->loadScene(fileName);
  return {};
}

void Renderer::cleanup() {
  engine->cleanup();
  engine = nullptr;
}

// Render Component
RenderComponent::RenderComponent(RenderEngine* renderEngine) {
  this->engine = renderEngine;
}
