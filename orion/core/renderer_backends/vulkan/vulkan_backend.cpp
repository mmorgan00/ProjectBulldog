#include "vulkan_backend.hpp"

#include "../../../util/logger.hpp"
#include "VkBootstrap.h"
#include "core/engine_types.hpp"
#include <SDL.h>
#include <SDL_vulkan.h>
#include <iostream>
#include <vulkan/vulkan_core.h>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

bool VulkanRendererBackend::init(app_state &state) {
  init_inst(state);
  init_swapchain();
  init_commands();
  // TODO:
  // init_sync_structures();
  // init_descriptors();
  // init_pipelines();
  // init_imgui();
  // init_default_data();
  //
  return true;
}
DECLARE_LOG_CATEGORY(VULKAN_BACKEND)
void VulkanRendererBackend::init_inst(app_state &state) {
  // Init SDL
  // Initialize SDL
  // TODO: Tuck into a 'platform init'
  OE_LOG(VULKAN_BACKEND, INFO, "Initializing platform window");

  SDL_Init(SDL_INIT_VIDEO);
  SDL_SetRelativeMouseMode(SDL_TRUE);

  SDL_WindowFlags window_flags =
      (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
  _window = SDL_CreateWindow(state.appName.c_str(), SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED, _windowExtent.width,
                             _windowExtent.height, window_flags);

  // Initialize vulkan
  //> init_instance
  vkb::InstanceBuilder builder;

  // make the vulkan instance, with basic debug features
  auto inst_ret =
      builder.set_app_name("Project Bulldog")
          .request_validation_layers(VK_TRUE) // TODO: Make configurable
          .use_default_debug_messenger()
          .require_api_version(1, 3, 0)
          .build();

  vkb::Instance vkb_inst = inst_ret.value();
  if (!vkb_inst) {
    OE_LOG(VULKAN_BACKEND, FATAL,
           "Unable to initialize Vulkan instance! Exiting...");
    exit(1);
  }
  // grab the instance
  _instance = vkb_inst.instance;
  _debug_messenger = vkb_inst.debug_messenger;

  //< init_instance
  OE_LOG(VULKAN_BACKEND, TRACE, "Vulkan instance created");
  //
  //> init_device
  SDL_Vulkan_CreateSurface(_window, _instance, &_surface);

  OE_LOG(VULKAN_BACKEND, TRACE, "SDL-Vulkan surface created");
  // vulkan 1.3 features
  VkPhysicalDeviceVulkan13Features features{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
  features.dynamicRendering = true;
  features.synchronization2 = true;

  // vulkan 1.2 features
  VkPhysicalDeviceVulkan12Features features12{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
  features12.bufferDeviceAddress = true;
  features12.descriptorIndexing = true;

  // use vkbootstrap to select a gpu.
  // We want a gpu that can write to the SDL surface and supports vulkan 1.3
  // with the correct features
  vkb::PhysicalDeviceSelector selector{vkb_inst};
  vkb::PhysicalDevice physicalDevice = selector.set_minimum_version(1, 3)
                                           .set_required_features_13(features)
                                           .set_required_features_12(features12)
                                           .set_surface(_surface)
                                           .select()
                                           .value();
  OE_LOG(VULKAN_BACKEND, TRACE, "Physical device selected");

  // create the final vulkan device
  vkb::DeviceBuilder deviceBuilder{physicalDevice};

  vkb::Device vkbDevice = deviceBuilder.build().value();

  // Retrieve device properties to get the GPU name
  VkPhysicalDeviceProperties device_properties;
  vkGetPhysicalDeviceProperties(physicalDevice.physical_device,
                                &device_properties);
  OE_LOG(VULKAN_BACKEND, INFO, "Device selected : {}",
         device_properties.deviceName);

  // Get the VkDevice handle used in the rest of a vulkan application
  _device = vkbDevice.device;
  _chosenGPU = physicalDevice.physical_device;
  //< init_device

  //> init_queue
  // use vkbootstrap to get a Graphics queue
  _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
  _graphicsQueueFamily =
      vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
  //< init_queue
  OE_LOG(VULKAN_BACKEND, TRACE, "Queues selected");

  //> init allocator
  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.physicalDevice = _chosenGPU;
  allocatorInfo.device = _device;
  allocatorInfo.instance = _instance;
  allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
  vmaCreateAllocator(&allocatorInfo, &_allocator);
  //< init allocator
  _mainDeletionQueue.push_function([&]() { vmaDestroyAllocator(_allocator); });
}
void VulkanRendererBackend::create_swapchain(uint32_t width, uint32_t height) {
  vkb::SwapchainBuilder swapchainBuilder{_chosenGPU, _device, _surface};

  _swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

  vkb::Swapchain vkbSwapchain =
      swapchainBuilder
          //.use_default_format_selection()
          .set_desired_format(VkSurfaceFormatKHR{
              .format = _swapchainImageFormat,
              .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
          // use vsync present mode
          .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
          .set_desired_extent(width, height)
          .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
          .build()
          .value();

  _swapchainExtent = vkbSwapchain.extent;
  // store swapchain and its related images
  _swapchain = vkbSwapchain.swapchain;
  _swapchainImages = vkbSwapchain.get_images().value();
  _swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void VulkanRendererBackend::init_swapchain() {
  create_swapchain(_windowExtent.width, _windowExtent.height);

  // draw image size will match the window
  VkExtent3D drawImageExtent = {_windowExtent.width, _windowExtent.height, 1};

  // hardcoding the draw format to 32 bit float
  _drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
  _drawImage.imageExtent = drawImageExtent;

  VkImageUsageFlags drawImageUsages{};
  drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
  drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  VkImageCreateInfo rimg_info = vkinit::image_create_info(
      _drawImage.imageFormat, drawImageUsages, drawImageExtent);

  // for the draw image, we want to allocate it from gpu local memory
  VmaAllocationCreateInfo rimg_allocinfo = {};
  rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  rimg_allocinfo.requiredFlags =
      VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  // allocate and create the image
  vmaCreateImage(_allocator, &rimg_info, &rimg_allocinfo, &_drawImage.image,
                 &_drawImage.allocation, nullptr);

  // build a image-view for the draw image to use for rendering
  VkImageViewCreateInfo rview_info = vkinit::imageview_create_info(
      _drawImage.imageFormat, _drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

  VK_CHECK(
      vkCreateImageView(_device, &rview_info, nullptr, &_drawImage.imageView));

  _depthImage.imageFormat = VK_FORMAT_D32_SFLOAT;
  _depthImage.imageExtent = drawImageExtent;
  VkImageUsageFlags depthImageUsages{};
  depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

  VkImageCreateInfo dimg_info = vkinit::image_create_info(
      _depthImage.imageFormat, depthImageUsages, drawImageExtent);

  // allocate and create the image
  vmaCreateImage(_allocator, &dimg_info, &rimg_allocinfo, &_depthImage.image,
                 &_depthImage.allocation, nullptr);

  // build a image-view for the draw image to use for rendering
  VkImageViewCreateInfo dview_info = vkinit::imageview_create_info(
      _depthImage.imageFormat, _depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);

  VK_CHECK(
      vkCreateImageView(_device, &dview_info, nullptr, &_depthImage.imageView));

  OE_LOG(VULKAN_BACKEND, TRACE, "Swapchain created");
  // add to deletion queues
  _mainDeletionQueue.push_function(
      [_drawImageImageView = _drawImage.imageView,
       _drawImageImage = _drawImage.image, _allocator = _allocator,
       _device = _device, _drawImageAllocation = _drawImage.allocation,
       _depthImageImageView = _depthImage.imageView,
       _depthImageImage = _depthImage.image,
       _depthImageAllocation = _depthImage.allocation]() {
        vkDestroyImageView(_device, _drawImageImageView, nullptr);
        vmaDestroyImage(_allocator, _drawImageImage, _drawImageAllocation);

        vkDestroyImageView(_device, _depthImageImageView, nullptr);
        vmaDestroyImage(_allocator, _depthImageImage, _depthImageAllocation);
      });
}

void VulkanRendererBackend::init_commands() {
  // create a command pool for commands submitted to the graphics queue.
  // we also want the pool to allow for resetting of individual command buffers
  VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(
      _graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

  for (int i = 0; i < FRAME_OVERLAP; i++) {
    VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr,
                                 &_frames[i]._commandPool));

    // allocate the default command buffer that we will use for rendering
    VkCommandBufferAllocateInfo cmdAllocInfo =
        vkinit::command_buffer_allocate_info(_frames[i]._commandPool, 1);

    VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo,
                                      &_frames[i]._mainCommandBuffer));
  }
  VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr,
                               &_immCommandPool));

  // allocate the command buffer for immediate submits
  VkCommandBufferAllocateInfo cmdAllocInfo =
      vkinit::command_buffer_allocate_info(_immCommandPool, 1);

  VK_CHECK(
      vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_immCommandBuffer));

  OE_LOG(VULKAN_BACKEND, INFO, "Command buffers allocated");

  _mainDeletionQueue.push_function(
      [this]() { vkDestroyCommandPool(_device, _immCommandPool, nullptr); });
}

void VulkanRendererBackend::shutdown() {
  OE_LOG(VULKAN_BACKEND, INFO, "Shutting down vulkan renderer");
  _mainDeletionQueue.flush();
}
