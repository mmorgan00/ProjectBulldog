// Copyright 2025 Max Morgan

#include "core/render_engines/vulkan/vulkan_loaders.h"

#include <string>

#include "util/logger.h"

std::optional<std::vector<std::shared_ptr<MeshAsset>>> vkutil::loadMeshGLB(
    VulkanEngine* engine, std::string filePath) {
  OE_LOG(RENDERER, INFO, "Loading GLB {}", filePath);
  return {};
}
