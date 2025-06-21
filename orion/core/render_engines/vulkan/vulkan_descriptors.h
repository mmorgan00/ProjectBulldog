// Copyright 2025 Max Morgan

#ifndef ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_DESCRIPTORS_H_
#define ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_DESCRIPTORS_H_

#include <vector>
#include <span>

struct DescriptorLayoutBuilder {
  std::vector<VkDescriptorSetLayoutBinding> bindings;

  void add_binding(uint32_t binding, VkDescriptorType type);
  void clear();
  VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shaderStages,
                              void* pNext = nullptr,
                              VkDescriptorSetLayoutCreateFlags flags = 0);
};

struct DescriptorAllocator {
  struct PoolSizeRatio {
    VkDescriptorType type;
    float ratio;
  };

  VkDescriptorPool pool;

  void init_pool(VkDevice device, uint32_t maxSets,
                 std::span<PoolSizeRatio> poolRatios);
  void clear_descriptors(VkDevice device);
  void destroy_pool(VkDevice device);

  VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout);
};

#endif  // ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_DESCRIPTORS_H_
