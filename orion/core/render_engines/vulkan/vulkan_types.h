// Copyright 2025 Max Morgan
#ifndef ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_TYPES_H_
#define ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_TYPES_H_

#include <util/logger.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>
/**
 * @brief Holds the data specific to a single frame
 * @detail Includes command objects
 */
typedef struct FrameData {
  VkCommandPool _commandPool;
  VkCommandBuffer _mainCommandBuffer;
  VkFence _renderFence;
} FrameData;

DECLARE_LOG_CATEGORY(VULKAN_VALIDATION)

#define VK_CHECK(x)                                                 \
  do {                                                              \
    VkResult err = x;                                               \
    if (err) {                                                      \
      OE_LOG(VULKAN_VALIDATION, ERROR, "Detected Vulkan error: {}", \
             string_VkResult(err));                                 \
      abort();                                                      \
    }                                                               \
  } while (0)
#endif  // ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_TYPES_H_
