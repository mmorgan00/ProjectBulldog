#pragma once

#include <vulkan/vulkan.h>
#include <fmt/core.h>
#include <vk_mem_alloc.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>



struct AllocatedImage {
  VkImage image;
  VkImageView imageView;
  VmaAllocation allocation;
  VkExtent3D imageExtent;
  VkFormat imageFormat;
};

struct GPUDrawPushConstants {
  glm::mat4 worldMatrix;
  VkDeviceAddress vertexBuffer;
};



#define VK_CHECK(x)                                                  \
  do {                                                               \
    VkResult err = x;                                                \
    if (err) {                                                       \
      fmt::print("Detected Vulkan error: {}", string_VkResult(err)); \
      abort();                                                       \
    }                                                                \
  } while (0)
