// Copyright 2025 Max Morgan

#ifndef ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_LOADERS_H_
#define ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_LOADERS_H_


#include "core/render_engines//vulkan/vulkan_engine.h"
#include "core/render_engines/vulkan/vulkan_types.h"

#include <filesystem>

namespace vkutil {
std::optional<std::vector<std::shared_ptr<MeshAsset>>> loadMeshGLB(
    VulkanEngine* engine, std::filesystem::path filePath);
};

#endif  // ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_LOADERS_H_
