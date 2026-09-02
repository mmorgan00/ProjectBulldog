// Copyright 2025 Max Morgan

#include "orion/core/renderer.h"

#include <fmt/ranges.h>

#include <cstdint>
#include <memory>
#include <string>

#include "orion/core/render_engines/vulkan/vulkan_engine.h"
#include "orion/core/renderer_types.h"
#include "orion/core/resource_types/static_mesh.h"
#include "orion/entity/camera.h"
#include "orion/util/logger.h"

void Renderer::init(AppState& state) {
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
void Renderer::draw(engine::DrawContext ctx) { engine->draw(ctx); }

/**
 * brief Sets a designated camera to be used as the Renderer view matrix
 */
void Renderer::set_camera(Camera* camera) { engine->set_camera(camera); }
// /**
//  * @detail loads a single object from file to add to the scene graph
//  */
// std::shared_ptr<RenderComponent> Renderer::loadScene(
//     std::string_view fileName) {
//   // TODO: Properly implement
//   this->engine->loadScene(fileName);
//   return {};
// }

void Renderer::cleanup() {
  engine->cleanup();
  engine = nullptr;
}

RenderObject* Renderer::loadStaticMesh(StaticMesh* mesh) {
  // 1. Convert static mesh to opaque render object
  engine::MeshAsset new_asset;
  new_asset.name = mesh->name;
  new_asset.meshBuffers.vertexBuffer = mesh->vertices;
  new_asset.meshBuffers.indexBuffer = mesh->indices;
  OE_LOG(RENDERER, DEBUG, "Loading {} indices to renderer",
         mesh->indices.size());
  OE_LOG(RENDERER, DEBUG, "Loading {} index data to renderer", mesh->indices);
  engine::Surface surf{.startIndex = 0,
                       .count = static_cast<uint32_t>(mesh->indices.size()),
                       .material = nullptr};
  new_asset.surfaces = std::vector<engine::Surface>{surf};
  RenderObject* robj = this->engine->uploadMesh(new_asset);
  return robj;
}
