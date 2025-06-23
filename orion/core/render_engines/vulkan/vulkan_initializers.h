// Copyright 2025 Max Morgan

#ifndef ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_INITIALIZERS_H_
#define ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_INITIALIZERS_H_

namespace vkinit {

VkCommandPoolCreateInfo command_pool_create_info(
    uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags /*= 0*/);

VkCommandBufferAllocateInfo command_buffer_allocate_info(
    VkCommandPool pool, uint32_t count /*= 1*/);

VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags /*= 0*/);

VkSemaphoreCreateInfo semaphore_create_info(
    VkSemaphoreCreateFlags flags /*= 0*/);

//< Command Buffers
VkCommandBufferBeginInfo command_buffer_begin_info(
    VkCommandBufferUsageFlags flags /*= 0*/);

VkCommandBufferSubmitInfo command_buffer_submit_info(VkCommandBuffer cmd);
//> Command Buffers

VkImageSubresourceRange image_subresource_range(VkImageAspectFlags aspectMask);

//< Sync structures
VkSemaphoreSubmitInfo semaphore_submit_info(VkPipelineStageFlags2 stageMask,
                                            VkSemaphore semaphore);
//> Sync structures

VkSubmitInfo2 submit_info(VkCommandBufferSubmitInfo *cmd,
                          VkSemaphoreSubmitInfo *signalSemaphoreInfo,
                          VkSemaphoreSubmitInfo *waitSemaphoreInfo);

//< Images
VkImageCreateInfo image_create_info(VkFormat format,
                                    VkImageUsageFlags usageFlags,
                                    VkExtent3D extent);
VkImageViewCreateInfo imageview_create_info(VkFormat format, VkImage image,
                                            VkImageAspectFlags aspectFlags);

// Pipelines

VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(
    VkShaderStageFlagBits stage, VkShaderModule shaderModule);

}  // namespace vkinit

#endif  // ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_INITIALIZERS_H_
