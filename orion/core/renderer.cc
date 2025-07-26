// Copyright 2025 Max Morgan

#include "core/renderer.h"

#include <memory>
#include <string>

#include "core/render_engines/vulkan/vulkan_engine.h"
#include "util/logger.h"

void Renderer::init(app_state &state) {
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

void Renderer::draw() {
  // TODO: Platform driven resize
  engine->draw();
}

/**
 * @detail loads a single object outside of a scene graph
 */
std::shared_ptr<RenderComponent> Renderer::loadObject() {
  return this->engine->loadObject();
}

void Renderer::cleanup() {
  engine->cleanup();
  engine = nullptr;
}

/**
 * @detail Loads the scene data and sends it to backend
 */
void Renderer::loadScene(std::string sceneName) {
  // Obtain scene file
  simdjson::ondemand::parser parser;
  OE_LOG(RENDERER, INFO, "Loading scene file ../../scenes/{}.json", sceneName);
  simdjson::padded_string json = simdjson::padded_string::load(
      fmt::format("../../scenes/{}.json", sceneName));
  simdjson::ondemand::document config = parser.iterate(json);
  try {
    OE_LOG(RENDERER, INFO, "Loaded scene with BG type {}",
           config["background"]["type"].get_string().value());
  } catch (std::exception e) {
    OE_LOG(RENDERER, INFO, "Failed to load scene!");
  }
  loadObject();
}

// Render Component
RenderComponent::RenderComponent(RenderEngine* renderEngine) {
  this->engine = renderEngine;
}
