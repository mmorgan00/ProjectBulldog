// Copyright 2025 Max Morgan
#ifndef ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_TYPES_H_
#define ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_TYPES_H_

#include <util/containers.h>
#include <util/logger.h>
#include <vk_mem_alloc.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include "render_engines/vulkan/vulkan_descriptors.h"

/**
 * @brief Scene graph
 */
typedef struct Scene {
} Scene;

/**
 * @brief Scene node
 */
typedef struct SceneNode {
} SceneNode;
/**
 * @brief Holds the data specific to a single frame
 * @detail Includes command objects
 */
typedef struct FrameData {
  VkCommandPool _commandPool;
  VkCommandBuffer _mainCommandBuffer;
  VkFence _renderFence;
  DeletionQueue _deletionQueue;
  DynamicDescriptorAllocator _frameDescriptors;
} FrameData;

struct AllocatedImage {
  VkImage image;
  VkImageView imageView;
  VmaAllocation allocation;
  VkExtent3D imageExtent;
  VkFormat imageFormat;
};

// Handle for an allocated buffer on GPU
struct AllocatedBuffer {
  VkBuffer buffer;           // The actual buffer
  VmaAllocation allocation;  // Memory allocation, needed for cleanup
  VmaAllocationInfo info;    // Allocation metadata, needed for cleanup
};

struct Vertex {
  glm::vec3 position;
  float uv_x;
  glm::vec3 normal;
  float uv_y;
  glm::vec4 color;
};

// holds the resources needed for a mesh

struct GPUMeshBuffers {
  AllocatedBuffer indexBuffer;
  AllocatedBuffer vertexBuffer;
  VkDeviceAddress vertexBufferAddress;
};

struct GeoSurface {
  uint32_t startIndex;
  uint32_t count;
};

struct MeshAsset {
  std::string name;
  std::vector<GeoSurface> surfaces;
  GPUMeshBuffers meshBuffers;
};

// push constants for our mesh object draws
struct GPUDrawPushConstants {
  glm::mat4 worldMatrix;
  VkDeviceAddress vertexBuffer;
};

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
