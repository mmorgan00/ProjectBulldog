// Copyright 2025 Max Morgan

#ifndef ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_IMAGES_H_
#define ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_IMAGES_H_
#include <vulkan/vulkan.h>

namespace vkutil {
void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
}
#endif  // ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_IMAGES_H_
