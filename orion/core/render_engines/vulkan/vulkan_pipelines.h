// Copyright 2025 Max Morgan

#ifndef ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_PIPELINES_H_
#define ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_PIPELINES_H_

namespace vkutil {

bool load_shader_module(const char* filePath, VkDevice device,
                        VkShaderModule* outShaderModule);

}
#endif  // ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_PIPELINES_H_
