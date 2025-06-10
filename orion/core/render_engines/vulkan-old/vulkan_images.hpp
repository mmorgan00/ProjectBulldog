
#pragma once

#include <vulkan/vulkan.h>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/parser.hpp>
#include <fastgltf/tools.hpp>
#include "vulkan_backend.hpp"
#include "core/engine_types.hpp"

namespace vkutil {

void transition_image(VkCommandBuffer cmd, VkImage image,
                      VkImageLayout currentLayout, VkImageLayout newLayout);
void copy_image_to_image(VkCommandBuffer cmd, VkImage source,
                         VkImage destination, VkExtent2D srcSize,
                         VkExtent2D dstSize);

void generate_mipmaps(VkCommandBuffer cmd, VkImage image, VkExtent2D imageSize);
std::optional<AllocatedImage> load_image(VulkanRendererBackend* engine, fastgltf::Asset& asset, fastgltf::Image& image);

}
