#include <vk_images.h>
#include <vk_initializers.h>
#include <vulkan/vulkan_core.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//> transition
#include <vk_initializers.h>

void vkutil::transition_image(VkCommandBuffer cmd, VkImage image,
                              VkImageLayout currentLayout,
                              VkImageLayout newLayout) {
  VkImageMemoryBarrier2 imageBarrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
  imageBarrier.pNext = nullptr;

  imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
  imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  imageBarrier.dstAccessMask =
      VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

  imageBarrier.oldLayout = currentLayout;
  imageBarrier.newLayout = newLayout;

  VkImageAspectFlags aspectMask =
      (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
          ? VK_IMAGE_ASPECT_DEPTH_BIT
          : VK_IMAGE_ASPECT_COLOR_BIT;
  imageBarrier.subresourceRange = vkinit::image_subresource_range(aspectMask);
  imageBarrier.image = image;

  VkDependencyInfo depInfo{};
  depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  depInfo.pNext = nullptr;

  depInfo.imageMemoryBarrierCount = 1;
  depInfo.pImageMemoryBarriers = &imageBarrier;

  vkCmdPipelineBarrier2(cmd, &depInfo);
}
//< transition
//> copyimg
void vkutil::copy_image_to_image(VkCommandBuffer cmd, VkImage source,
                                 VkImage destination, VkExtent2D srcSize,
                                 VkExtent2D dstSize) {
  VkImageBlit2 blitRegion{.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
                          .pNext = nullptr};

  blitRegion.srcOffsets[1].x = srcSize.width;
  blitRegion.srcOffsets[1].y = srcSize.height;
  blitRegion.srcOffsets[1].z = 1;

  blitRegion.dstOffsets[1].x = dstSize.width;
  blitRegion.dstOffsets[1].y = dstSize.height;
  blitRegion.dstOffsets[1].z = 1;

  blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  blitRegion.srcSubresource.baseArrayLayer = 0;
  blitRegion.srcSubresource.layerCount = 1;
  blitRegion.srcSubresource.mipLevel = 0;

  blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  blitRegion.dstSubresource.baseArrayLayer = 0;
  blitRegion.dstSubresource.layerCount = 1;
  blitRegion.dstSubresource.mipLevel = 0;

  VkBlitImageInfo2 blitInfo{.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
                            .pNext = nullptr};
  blitInfo.dstImage = destination;
  blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  blitInfo.srcImage = source;
  blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  blitInfo.filter = VK_FILTER_LINEAR;
  blitInfo.regionCount = 1;
  blitInfo.pRegions = &blitRegion;

  vkCmdBlitImage2(cmd, &blitInfo);
} //< copyimg
//> mipgen
void vkutil::generate_mipmaps(VkCommandBuffer cmd, VkImage image,
                              VkExtent2D imageSize) {
  int mipLevels =
      int(std::floor(std::log2(std::max(imageSize.width, imageSize.height)))) +
      1;
  for (int mip = 0; mip < mipLevels; mip++) {
    VkExtent2D halfSize = imageSize;
    halfSize.width /= 2;
    halfSize.height /= 2;

    VkImageMemoryBarrier2 imageBarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2, .pNext = nullptr};

    imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.dstAccessMask =
        VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrier.subresourceRange = vkinit::image_subresource_range(aspectMask);
    imageBarrier.subresourceRange.levelCount = 1;
    imageBarrier.subresourceRange.baseMipLevel = mip;
    imageBarrier.image = image;

    VkDependencyInfo depInfo{.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                             .pNext = nullptr};
    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers = &imageBarrier;

    vkCmdPipelineBarrier2(cmd, &depInfo);

    if (mip < mipLevels - 1) {
      VkImageBlit2 blitRegion{.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
                              .pNext = nullptr};

      blitRegion.srcOffsets[1].x = imageSize.width;
      blitRegion.srcOffsets[1].y = imageSize.height;
      blitRegion.srcOffsets[1].z = 1;

      blitRegion.dstOffsets[1].x = halfSize.width;
      blitRegion.dstOffsets[1].y = halfSize.height;
      blitRegion.dstOffsets[1].z = 1;

      blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      blitRegion.srcSubresource.baseArrayLayer = 0;
      blitRegion.srcSubresource.layerCount = 1;
      blitRegion.srcSubresource.mipLevel = mip;

      blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      blitRegion.dstSubresource.baseArrayLayer = 0;
      blitRegion.dstSubresource.layerCount = 1;
      blitRegion.dstSubresource.mipLevel = mip + 1;

      VkBlitImageInfo2 blitInfo{.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
                                .pNext = nullptr};
      blitInfo.dstImage = image;
      blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      blitInfo.srcImage = image;
      blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      blitInfo.filter = VK_FILTER_LINEAR;
      blitInfo.regionCount = 1;
      blitInfo.pRegions = &blitRegion;

      vkCmdBlitImage2(cmd, &blitInfo);

      imageSize = halfSize;
    }
  }

  // transition all mip levels into the final read_only layout
  transition_image(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
//< mipgen

//> loading
std::optional<AllocatedImage> vkutil::load_image(VulkanEngine *engine,
                                                 fastgltf::Asset &asset,
                                                 fastgltf::Image &image) {
  AllocatedImage newImage{};

  int width, height, nrChannels;

  fmt::println("Load image called for {}", image.name);
  // fmt::println("Searching path to load image ");
  std::visit(
      fastgltf::visitor{
          [](auto &arg) {
            fmt::println("Received image spec type that couldn't be handled");
          },
          [&](fastgltf::sources::URI &filePath) {
            assert(filePath.fileByteOffset ==
                   0); // We don't support offsets with stbi.
            assert(filePath.uri.isLocalPath()); // We're only capable of loading
                                                // local files.

            const std::string path(filePath.uri.path().begin(),
                                   filePath.uri.path().end()); // Thanks C++.
            unsigned char *data =
                stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
            if (data) {
              VkExtent3D imagesize;
              imagesize.width = width;
              imagesize.height = height;
              imagesize.depth = 1;

              newImage = engine->create_image(
                  data, imagesize, VK_FORMAT_R8G8B8A8_UNORM,
                  VK_IMAGE_USAGE_SAMPLED_BIT, false);

              stbi_image_free(data);
            } else {
              newImage = engine->_greyImage;
            }
          },
          [&](fastgltf::sources::Vector &vector) {
            const unsigned char *buffer =
                reinterpret_cast<const unsigned char *>(vector.bytes.data());
            unsigned char *data = stbi_load_from_memory(
                buffer, static_cast<int>(vector.bytes.size()), &width, &height,
                &nrChannels, 4);

            if (data) {
              VkExtent3D imagesize;
              imagesize.width = width;
              imagesize.height = height;
              imagesize.depth = 1;

              newImage = engine->create_image(
                  data, imagesize, VK_FORMAT_R8G8B8A8_UNORM,
                  VK_IMAGE_USAGE_SAMPLED_BIT, false);

              stbi_image_free(data);
            } else {
              newImage = engine->_greyImage;
            }
          },
          [&](fastgltf::sources::BufferView &view) {
            auto &bufferView = asset.bufferViews[view.bufferViewIndex];
            auto &buffer = asset.buffers[bufferView.bufferIndex];

            std::visit(fastgltf::visitor{
                           // We only care about VectorWithMime here, because we
                           // specify LoadExternalBuffers, meaning all buffers
                           // are already loaded into a vector.
                           [](auto &arg) {},
                           [&](fastgltf::sources::Vector &vector) {
                             const unsigned char *buffer =
                                 reinterpret_cast<const unsigned char *>(
                                     vector.bytes.data());
                             unsigned char *data = stbi_load_from_memory(
                                 buffer, static_cast<int>(vector.bytes.size()),
                                 &width, &height, &nrChannels, 4);

                             if (data) {
                               VkExtent3D imagesize;
                               imagesize.width = width;
                               imagesize.height = height;
                               imagesize.depth = 1;

                               newImage = engine->create_image(
                                   data, imagesize, VK_FORMAT_R8G8B8A8_UNORM,
                                   VK_IMAGE_USAGE_SAMPLED_BIT, false);

                               stbi_image_free(data);
                             } else {
                               fmt::println("Failed to load texture data, "
                                            "defaulting to error image");
                               VkExtent3D imagesize;
                               imagesize.width = 16;
                               imagesize.height = 16;
                               imagesize.depth = 1;
                               newImage = engine->create_image(
                                   engine->_greyImage.image,
                                   imagesize, VK_FORMAT_R8G8B8A8_UNORM,
                                   VK_IMAGE_USAGE_SAMPLED_BIT, false);
                             }
                           }},
                       buffer.data);
          },
      },
      image.data);

  // if any of the attempts to load the data failed, we havent written the image
  // so handle is null
  if (newImage.image == VK_NULL_HANDLE) {
    return {};
  } else {
    return newImage;
  }
}

//> loading
