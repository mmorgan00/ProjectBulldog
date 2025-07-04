// Copyright 2025 Max Morgan

#include "orion/core/render_engines/vulkan/vulkan_engine.h"

#include <SDL.h>
#include <SDL_vulkan.h>
#include <VkBootstrap.h>
#include <fmt/printf.h>
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <vector>

#include "core/engine_types.h"
#include "orion/core/render_engines/vulkan/vulkan_images.h"
#include "orion/core/render_engines/vulkan/vulkan_initializers.h"
#include "orion/core/render_engines/vulkan/vulkan_pipelines.h"
#include "orion/core/render_engines/vulkan/vulkan_types.h"
#include "util/logger.h"

#define VMA_IMPLEMENTATION
#include "third_party/vma/vk_mem_alloc.h"

VulkanEngine* loadedEngine = nullptr;

bool VulkanEngine::init(app_state& state) {
  // We initialize SDL and create a window with it.
  SDL_Init(SDL_INIT_VIDEO);

  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

  _window = SDL_CreateWindow(state.appName.c_str(), SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED, _windowExtent.width,
                             _windowExtent.height, window_flags);
  init_vulkan(state);
  init_swapchain();
  init_commands();
  init_sync_structures();
  init_descriptors();
  init_pipelines();
  _isInitialized = true;
  return true;
}

void VulkanEngine::cleanup() {
  if (_isInitialized) {
    vkDeviceWaitIdle(_device);

    for (int i = 0; i < MAX_CONCURRENT_FRAMES; i++) {
      vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);

      // destroy sync objects
      vkDestroyFence(_device, _frames[i]._renderFence, nullptr);
    }
    for (size_t i = 0; i < presentCompleteSemaphores.size(); i++) {
      vkDestroySemaphore(_device, presentCompleteSemaphores[i], nullptr);
    }
    for (size_t i = 0; i < renderCompleteSemaphores.size(); i++) {
      vkDestroySemaphore(_device, renderCompleteSemaphores[i], nullptr);
    }

    _mainDeletionQueue.flush();
    destroy_swapchain();
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkDestroyDevice(_device, nullptr);

    vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
    vkDestroyInstance(_instance, nullptr);
    SDL_DestroyWindow(_window);
  }

  // clear engine pointer
  loadedEngine = nullptr;
}

void VulkanEngine::loadScene() {
}

void VulkanEngine::draw_background(VkCommandBuffer cmd) {
  // bind the gradient drawing compute pipeline
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _gradientPipeline);

  // bind the descriptor set containing the draw image for the compute pipeline
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                          _gradientPipelineLayout, 0, 1, &_drawImageDescriptors,
                          0, nullptr);

  // execute the compute pipeline dispatch. We are using 16x16 workgroup size so
  // we need to divide by it
  vkCmdDispatch(cmd, std::ceil(_drawImage.imageExtent.width / 16.0),
                std::ceil(_drawImage.imageExtent.height / 16.0), 1);
}

void VulkanEngine::draw_geometry(VkCommandBuffer cmd) {
  // begin a render pass  connected to our draw image
  VkRenderingAttachmentInfo colorAttachment = vkinit::attachment_info(
      _drawImage.imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  VkRenderingInfo renderInfo =
      vkinit::rendering_info(_drawExtent, &colorAttachment, nullptr);
  vkCmdBeginRendering(cmd, &renderInfo);

  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _defaultPipeline);

  // set dynamic viewport and scissor
  VkViewport viewport = {};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = _drawExtent.width;
  viewport.height = _drawExtent.height;
  viewport.minDepth = 0.f;
  viewport.maxDepth = 1.f;

  vkCmdSetViewport(cmd, 0, 1, &viewport);

  VkRect2D scissor = {};
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  scissor.extent.width = _drawExtent.width;
  scissor.extent.height = _drawExtent.height;

  vkCmdSetScissor(cmd, 0, 1, &scissor);

  // launch a draw command to draw 3 vertices
  vkCmdDraw(cmd, 3, 1, 0, 0);

  vkCmdEndRendering(cmd);
}

void VulkanEngine::draw() {
  //> draw_1
  // wait until the gpu has finished rendering the last frame. Timeout of 1
  // second
  VK_CHECK(vkWaitForFences(_device, 1, &get_current_frame()._renderFence, true,
                           1000000000));
  VK_CHECK(vkResetFences(_device, 1, &get_current_frame()._renderFence));
  get_current_frame()._deletionQueue.flush();  // Clean up any per-frame objects
                                               // created last frame.
  //< draw_1

  //> draw_2
  // request image from the swapchain
  uint32_t swapchainImageIndex;
  VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000,
                                 presentCompleteSemaphores[currentSemaphore],
                                 nullptr, &swapchainImageIndex));
  // Set our draw image size based on the image we obtained
  _drawExtent.height =
      std::min(_swapchainExtent.height, _drawImage.imageExtent.height);
  _drawExtent.width =
      std::min(_swapchainExtent.width, _drawImage.imageExtent.width);
  //< draw_2

  //> draw_3
  // naming it cmd for shorter writing
  VkCommandBuffer cmd = get_current_frame()._mainCommandBuffer;

  // now that we are sure that the commands finished executing, we can safely
  // reset the command buffer to begin recording again.
  VK_CHECK(vkResetCommandBuffer(cmd, 0));

  // begin the command buffer recording. We will use this command buffer exactly
  // once, so we want to let vulkan know that
  VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

  // transition our main draw image into general layout so we can write into it
  // we will overwrite it all so we dont care about what was the older layout
  vkutil::transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_GENERAL);

  draw_background(cmd);

  vkutil::transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_GENERAL,
                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  draw_geometry(cmd);

  // transition the draw image and the swapchain image into their correct
  // transfer layouts
  vkutil::transition_image(cmd, _drawImage.image,
                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  vkutil::transition_image(cmd, _swapchainImages[swapchainImageIndex],
                           VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  // execute a copy from the draw image into the swapchain
  vkutil::copy_image_to_image(cmd, _drawImage.image,
                              _swapchainImages[swapchainImageIndex],
                              _drawExtent, _swapchainExtent);

  // set swapchain image layout to Present so we can show it on the screen
  vkutil::transition_image(cmd, _swapchainImages[swapchainImageIndex],
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  // finalize the command buffer (we can no longer add commands, but it can now
  // be executed)
  VK_CHECK(vkEndCommandBuffer(cmd));  //< draw_4

  //> draw_5
  // prepare the submission to the queue.
  // we want to wait on the _presentSemaphore, as that semaphore is signaled
  // when the swapchain is ready we will signal the _renderSemaphore, to signal
  // that rendering has finished

  VkCommandBufferSubmitInfo cmdinfo = vkinit::command_buffer_submit_info(cmd);

  VkSemaphoreSubmitInfo waitInfo = vkinit::semaphore_submit_info(
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
      presentCompleteSemaphores[currentSemaphore]);
  VkSemaphoreSubmitInfo signalInfo =
      vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
                                    renderCompleteSemaphores[currentSemaphore]);

  VkSubmitInfo2 submit = vkinit::submit_info(&cmdinfo, &signalInfo, &waitInfo);

  // submit command buffer to the queue and execute it.
  //  _renderFence will now block until the graphic commands finish execution
  VK_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submit,
                          get_current_frame()._renderFence));
  //< draw_5
  //
  //> draw_6
  // prepare present
  // this will put the image we just rendered to into the visible window.
  // we want to wait on the _renderSemaphore for that,
  // as its necessary that drawing commands have finished before the image is
  // displayed to the user
  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = nullptr;
  presentInfo.pSwapchains = &_swapchain;
  presentInfo.swapchainCount = 1;

  presentInfo.pWaitSemaphores = &renderCompleteSemaphores[currentSemaphore];
  presentInfo.waitSemaphoreCount = 1;

  presentInfo.pImageIndices = &swapchainImageIndex;

  VK_CHECK(vkQueuePresentKHR(_graphicsQueue, &presentInfo));

  // increase the number of frames drawn
  _frameNumber++;
  //< draw_6
  // Select the next frame to render to, based on the max. no. of concurrent
  // frames
  currentFrame = (currentFrame + 1) % MAX_CONCURRENT_FRAMES;
  // Similar for the semaphores, which need to be unique to the swapchain images
  currentSemaphore = (currentSemaphore + 1) % _swapchainImages.size();
}

//< Initializations
void VulkanEngine::init_vulkan(app_state& state) {
  vkb::InstanceBuilder builder;

  // make the vulkan instance, with basic debug features
  auto inst_ret = builder
                      .set_app_name(state.appName.c_str())
#ifdef NDEBUG
                      .request_validation_layers(false)
#else
                      .request_validation_layers(true)
#endif

                      .use_default_debug_messenger()
                      .require_api_version(1, 3, 0)
                      .build();

  vkb::Instance vkb_inst = inst_ret.value();

  // grab the instance
  _instance = vkb_inst.instance;
  _debug_messenger = vkb_inst.debug_messenger;

  SDL_Vulkan_CreateSurface(_window, _instance, &_surface);

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

  // create the final vulkan device
  vkb::DeviceBuilder deviceBuilder{physicalDevice};

  vkb::Device vkbDevice = deviceBuilder.build().value();

  // Get the VkDevice handle used in the rest of a vulkan application
  _device = vkbDevice.device;
  _chosenGPU = physicalDevice.physical_device;
  // use vkbootstrap to get a Graphics queue
  _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
  _graphicsQueueFamily =
      vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.physicalDevice = _chosenGPU;
  allocatorInfo.device = _device;
  allocatorInfo.instance = _instance;
  allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
  vmaCreateAllocator(&allocatorInfo, &_allocator);

  _mainDeletionQueue.push_function([&]() { vmaDestroyAllocator(_allocator); });
}

void VulkanEngine::init_commands() {
  // create a command pool for commands submitted to the graphics queue.
  // we also want the pool to allow for resetting of individual command buffers
  OE_LOG(VULKAN_ENGINE, INFO, "Initializing command buffers");
  VkCommandPoolCreateInfo commandPoolInfo = {};
  commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolInfo.pNext = nullptr;
  commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  commandPoolInfo.queueFamilyIndex = _graphicsQueueFamily;

  for (int i = 0; i < MAX_CONCURRENT_FRAMES; i++) {
    VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr,
                                 &_frames[i]._commandPool));

    // allocate the default command buffer that we will use for rendering
    VkCommandBufferAllocateInfo cmdAllocInfo =
        vkinit::command_buffer_allocate_info(_frames[i]._commandPool, 1);

    VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo,
                                      &_frames[i]._mainCommandBuffer));
  }
}
void VulkanEngine::init_sync_structures() {
  VkFenceCreateInfo fenceCreateInfo =
      vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
  VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info(0);

  for (int i = 0; i < MAX_CONCURRENT_FRAMES; i++) {
    VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr,
                           &_frames[i]._renderFence));
  }
  presentCompleteSemaphores.resize(_swapchainImages.size());
  renderCompleteSemaphores.resize(_swapchainImages.size());

  for (int i = 0; i < _swapchainImages.size(); i++) {
    // Semaphore used to ensure that image presentation is complete before
    // starting to submit again
    VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr,
                               &presentCompleteSemaphores[i]));
    // Semaphore used to ensure that all commands submitted have been finished
    // before submitting the image to the queue
    VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr,
                               &renderCompleteSemaphores[i]));
  }
}

void VulkanEngine::init_descriptors() {
  // create a descriptor pool that will hold 10 sets with 1 image each
  std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}};

  globalDescriptorAllocator.init_pool(_device, 10, sizes);

  // make the descriptor set layout for our compute draw
  {
    DescriptorLayoutBuilder builder;
    // Add an image on binding 0
    builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    // This descriptor is designed for a compute pass
    _drawImageDescriptorLayout =
        builder.build(_device, VK_SHADER_STAGE_COMPUTE_BIT);
  }

  // allocate a descriptor set for our draw image
  _drawImageDescriptors =
      globalDescriptorAllocator.allocate(_device, _drawImageDescriptorLayout);

  VkDescriptorImageInfo imgInfo{};
  imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
  imgInfo.imageView = _drawImage.imageView;

  VkWriteDescriptorSet drawImageWrite = {};
  drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  drawImageWrite.pNext = nullptr;

  drawImageWrite.dstBinding = 0;
  drawImageWrite.dstSet = _drawImageDescriptors;
  drawImageWrite.descriptorCount = 1;
  drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  drawImageWrite.pImageInfo = &imgInfo;

  vkUpdateDescriptorSets(_device, 1, &drawImageWrite, 0, nullptr);

  // make sure both the descriptor allocator and the new layout get cleaned up
  // properly
  _mainDeletionQueue.push_function([&]() {
    globalDescriptorAllocator.destroy_pool(_device);

    vkDestroyDescriptorSetLayout(_device, _drawImageDescriptorLayout, nullptr);
  });
}

void VulkanEngine::init_pipelines() {
  init_background_pipeline();
  init_default_pipeline();
}

void VulkanEngine::init_default_pipeline() {
  VkShaderModule vertShader;
  if (!vkutil::load_shader_module("../../assets/shaders/default.vert.spv",
                                  _device, &vertShader)) {
    OE_LOG(VULKAN_ENGINE, FATAL,
           "Failed to load default pipeline fragment shader");
  }
  VkShaderModule fragShader;
  if (!vkutil::load_shader_module("../../assets/shaders/default.frag.spv",
                                  _device, &fragShader)) {
    OE_LOG(VULKAN_ENGINE, FATAL,
           "Failed to load default pipeline fragment shader");
  }
  VkPipelineLayoutCreateInfo pipeline_layout_info =
      vkinit::pipeline_layout_create_info();
  VK_CHECK(vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr,
                                  &_defaultPipelineLayout));

  PipelineBuilder pipelineBuilder;

  // use the triangle layout we created
  pipelineBuilder._pipelineLayout = _defaultPipelineLayout;
  // connecting the vertex and pixel shaders to the pipeline
  pipelineBuilder.set_shaders(vertShader, fragShader);
  // it will draw triangles
  pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  // filled triangles
  pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
  // no backface culling
  pipelineBuilder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
  // no multisampling
  pipelineBuilder.set_multisampling_none();
  // no blending
  pipelineBuilder.disable_blending();
  // no depth testing
  pipelineBuilder.disable_depthtest();

  // connect the image format we will draw into, from draw image
  pipelineBuilder.set_color_attachment_format(_drawImage.imageFormat);
  pipelineBuilder.set_depth_format(VK_FORMAT_UNDEFINED);

  // finally build the pipeline
  _defaultPipeline = pipelineBuilder.build_pipeline(_device);

  // clean structures
  vkDestroyShaderModule(_device, fragShader, nullptr);
  vkDestroyShaderModule(_device, vertShader, nullptr);

  _mainDeletionQueue.push_function([&]() {
    vkDestroyPipelineLayout(_device, _defaultPipelineLayout, nullptr);
    vkDestroyPipeline(_device, _defaultPipeline, nullptr);
  });
}

void VulkanEngine::init_background_pipeline() {
  // build layout first.
  VkPipelineLayoutCreateInfo computeLayout{};
  computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  computeLayout.pNext = nullptr;
  computeLayout.pSetLayouts = &_drawImageDescriptorLayout;
  computeLayout.setLayoutCount = 1;

  VK_CHECK(vkCreatePipelineLayout(_device, &computeLayout, nullptr,
                                  &_gradientPipelineLayout));

  VkShaderModule computeDrawShader;
  // TODO: Load this properly?
  if (!vkutil::load_shader_module("../../assets/shaders/gradient.comp.spv", _device,
                                  &computeDrawShader)) {
    fmt::print("Error when building the compute shader \n");
  }

  VkPipelineShaderStageCreateInfo stageinfo{};
  stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stageinfo.pNext = nullptr;
  stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  stageinfo.module = computeDrawShader;
  stageinfo.pName = "main";

  VkComputePipelineCreateInfo computePipelineCreateInfo{};
  computePipelineCreateInfo.sType =
      VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  computePipelineCreateInfo.pNext = nullptr;
  computePipelineCreateInfo.layout = _gradientPipelineLayout;
  computePipelineCreateInfo.stage = stageinfo;

  VK_CHECK(vkCreateComputePipelines(_device, VK_NULL_HANDLE, 1,
                                    &computePipelineCreateInfo, nullptr,
                                    &_gradientPipeline));

  vkDestroyShaderModule(_device, computeDrawShader, nullptr);

  _mainDeletionQueue.push_function([&]() {
    vkDestroyPipelineLayout(_device, _gradientPipelineLayout, nullptr);
    vkDestroyPipeline(_device, _gradientPipeline, nullptr);
  });
}

void VulkanEngine::create_swapchain(uint32_t width, uint32_t height) {
  vkb::SwapchainBuilder swapchainBuilder{_chosenGPU, _device, _surface};

  _swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

  vkb::Swapchain vkbSwapchain =
      swapchainBuilder
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

void VulkanEngine::init_swapchain() {
  create_swapchain(_windowExtent.width, _windowExtent.height);

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

  // add to deletion queues
  _mainDeletionQueue.push_function([this]() {
    vkDestroyImageView(_device, _drawImage.imageView, nullptr);
    vmaDestroyImage(_allocator, _drawImage.image, _drawImage.allocation);
  });
}

void VulkanEngine::destroy_swapchain() {
  vkDestroySwapchainKHR(_device, _swapchain, nullptr);

  // destroy swapchain resources
  for (int i = 0; i < _swapchainImageViews.size(); i++) {
    vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
  }
}

//> Initializations
//
