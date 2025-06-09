#include "vulkan_backend.hpp"

#include "VkBootstrap.h"
#include "core/engine_types.hpp"
#include "util/logger.hpp"
#include "vulkan_images.hpp"
#include "vulkan_pipelines.hpp"
#include <SDL.h>
#include <SDL_vulkan.h>
#include <glm/gtx/transform.hpp>
#include <vulkan/vulkan_core.h>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

DECLARE_LOG_CATEGORY(VULKAN_BACKEND)
void VulkanRendererBackend::run() {

  VK_CHECK(vkWaitForFences(_device, 1, &get_current_frame()._renderFence, true,
                           1000000000));

  //< pt 1 Reset from previous frame and capture window size
  get_current_frame()._deletionQueue.flush();
  get_current_frame()._frameDescriptors.clear_pools(_device);
  _drawExtent.height =
      std::min(_swapchainExtent.height, _drawImage.imageExtent.height) *
      renderScale;
  _drawExtent.width =
      std::min(_swapchainExtent.width, _drawImage.imageExtent.width) *
      renderScale;

  VK_CHECK(vkResetFences(_device, 1, &get_current_frame()._renderFence));
  //< pt 1

  //> pt 2 request image from the swapchain
  uint32_t swapchainImageIndex;
  VkResult e = vkAcquireNextImageKHR(_device, _swapchain, 1000000000,
                                     get_current_frame()._swapchainSemaphore,
                                     nullptr, &swapchainImageIndex);
  // if (e == VK_ERROR_OUT_OF_DATE_KHR) {
  // resize_requested = true;
  // return;
  // } //< pt 2

  //> pt 3 prepare command buffer
  VkCommandBuffer cmd = get_current_frame()._mainCommandBuffer;

  // now that we are sure that the commands finished executing, we can safely
  // reset the command buffer to begin recording again.
  VK_CHECK(vkResetCommandBuffer(cmd, 0));

  // begin the command buffer recording. We will use this command buffer exactly
  // once, so we want to let vulkan know that
  VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  // pt 3

  //< Record command buffer
  VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

  // transition our main draw image into general layout so we can write into it
  // we will overwrite it all so we dont care about what was the older layout
  vkutil::transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_GENERAL);

  // draw_background(cmd);

  vkutil::transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_GENERAL,
                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  vkutil::transition_image(cmd, _depthImage.image, VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
  draw_geometry(cmd);

  // transtion the draw image and the swapchain image into their correct
  // transfer layouts
  vkutil::transition_image(cmd, _drawImage.image,
                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  vkutil::transition_image(cmd, _swapchainImages[swapchainImageIndex],
                           VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  // transfer layouts
  vkutil::transition_image(cmd, _swapchainImages[swapchainImageIndex],
                           VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  // execute a copy from the draw image into the swapchain
  vkutil::copy_image_to_image(cmd, _drawImage.image,
                              _swapchainImages[swapchainImageIndex],
                              _drawExtent, _swapchainExtent);

  // set swapchain image layout to Attachment Optimal so we can draw it
  vkutil::transition_image(cmd, _swapchainImages[swapchainImageIndex],
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  // draw imgui into the swapchain image
  // draw_imgui(cmd, _swapchainImageViews[swapchainImageIndex]);

  // set swapchain image layout to Present so we can draw it
  vkutil::transition_image(cmd, _swapchainImages[swapchainImageIndex],
                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                           VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  // finalize the command buffer (we can no longer add commands, but it can now
  // be executed)
  VK_CHECK(vkEndCommandBuffer(cmd)); //> end record command buffer

  //> draw_5
  // prepare the submission to the queue.
  // we want to wait on the _presentSemaphore, as that semaphore is signaled
  // when the swapchain is ready we will signal the _renderSemaphore, to signal
  // that rendering has finished

  VkCommandBufferSubmitInfo cmdinfo = vkinit::command_buffer_submit_info(cmd);

  VkSemaphoreSubmitInfo waitInfo = vkinit::semaphore_submit_info(
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
      get_current_frame()._swapchainSemaphore);
  VkSemaphoreSubmitInfo signalInfo =
      vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
                                    get_current_frame()._renderSemaphore);

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

  presentInfo.pWaitSemaphores = &get_current_frame()._renderSemaphore;
  presentInfo.waitSemaphoreCount = 1;

  presentInfo.pImageIndices = &swapchainImageIndex;

  VkResult presentResult = vkQueuePresentKHR(_graphicsQueue, &presentInfo);
  // if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
  //   resize_requested = true;
  // }
  // increase the number of frames drawn
  _frameNumber++;

  //< draw_6
}

void VulkanRendererBackend::draw_geometry(VkCommandBuffer cmd) {
  // allocate a new uniform buffer for the scene data
  // AllocatedBuffer gpuSceneDataBuffer =
  // 	create_buffer(sizeof(GPUSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
  // 		VMA_MEMORY_USAGE_CPU_TO_GPU);

  // add it to the deletion queue of this frame so it gets deleted once its been
  // used
  // get_current_frame()._deletionQueue.push_function(
  // [=, this]() { destroy_buffer(gpuSceneDataBuffer); });

  // write the buffer
  // GPUSceneData* sceneUniformData =
  // 	(GPUSceneData*)gpuSceneDataBuffer.allocation->GetMappedData();
  // *sceneUniformData = sceneData;

  // create a descriptor set that binds that buffer and update it
  // VkDescriptorSet globalDescriptor =
  // 	get_current_frame()._frameDescriptors.allocate(
  // 		_device, _gpuSceneDataDescriptorLayout);

  // DescriptorWriter writer;
  // writer.write_buffer(0, gpuSceneDataBuffer.buffer, sizeof(GPUSceneData), 0,
  // 	VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  // writer.update_set(_device, globalDescriptor);

  // begin a render pass  connected to our draw image
  VkRenderingAttachmentInfo colorAttachment = vkinit::attachment_info(
      _drawImage.imageView, nullptr, VK_IMAGE_LAYOUT_GENERAL);
  VkRenderingAttachmentInfo depthAttachment = vkinit::depth_attachment_info(
      _depthImage.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

  VkRenderingInfo renderInfo =
      vkinit::rendering_info(_windowExtent, &colorAttachment, &depthAttachment);
  vkCmdBeginRendering(cmd, &renderInfo);

  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _meshPipeline);

  // bind a texture
  //  VkDescriptorSet imageSet =
  //  get_current_frame()._frameDescriptors.allocate(_device,
  //  _singleImageDescriptorLayout);
  /**	{
                  DescriptorWriter writer;
                  writer.write_image(0, _errorCheckerboardImage.imageView,
     _defaultSamplerNearest, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
     VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

                  writer.update_set(_device, imageSet);
          }**/

  // vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
  // _meshPipelineLayout, 0, 1, &imageSet, 0, nullptr);

  glm::mat4 view = glm::translate(glm::vec3{0, 0, -5});
  // camera projection
  glm::mat4 projection = glm::perspective(
      glm::radians(70.f), (float)_drawExtent.width / (float)_drawExtent.height,
      10000.f, 0.1f);

  // invert the Y direction on projection matrix so that we are more similar
  // to opengl and gltf axis
  projection[1][1] *= -1;

  GPUDrawPushConstants push_constants;
  push_constants.worldMatrix = projection * view;
  // push_constants.vertexBuffer =
  // testMeshes[2]->meshBuffers.vertexBufferAddress;

  // draw lambda for each object.
  /** FROM OLD ENGINE
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
  draw.material->pipeline->pipeline); vkCmdBindDescriptorSets(cmd,
  VK_PIPELINE_BIND_POINT_GRAPHICS, draw.material->pipeline->layout, 0, 1,
  &globalDescriptor, 0, nullptr); vkCmdBindDescriptorSets(cmd,
  VK_PIPELINE_BIND_POINT_GRAPHICS, draw.material->pipeline->layout, 1, 1,
  &draw.material->materialSet, 0, nullptr);

                vkCmdBindIndexBuffer(cmd, draw.indexBuffer, 0,
  VK_INDEX_TYPE_UINT32);

                GPUDrawPushConstants pushConstants;
                pushConstants.vertexBuffer = draw.vertexBufferAddress;
                pushConstants.worldMatrix = draw.transform;
                vkCmdPushConstants(cmd, draw.material->pipeline->layout,
  VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);

                vkCmdDrawIndexed(cmd, draw.indexCount, 1, draw.firstIndex, 0,
  0);
                };

        for (auto& r : mainDrawContext.OpaqueSurfaces) {
                draw(r);
        }

        for (auto& r : mainDrawContext.TransparentSurfaces) {
                draw(r);
        }
        // we delete the draw commands now that we processed them
        mainDrawContext.OpaqueSurfaces.clear();
        mainDrawContext.TransparentSurfaces.clear();
  **/
  vkCmdEndRendering(cmd);
}

bool VulkanRendererBackend::init(app_state &state) {
  init_inst(state);
  init_swapchain();
  init_commands();
  init_sync_structures();
  init_descriptors(); // not rendering anything however.
  init_pipelines();
  // TODO:
  // init_imgui();
  OE_LOG(VULKAN_BACKEND, INFO, "Initializing defaullt data");
  init_default_data();

  //
  return true;
}
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

void VulkanRendererBackend::destroy_swapchain() {
  vkDestroySwapchainKHR(_device, _swapchain, nullptr);

  // destroy swapchain resources
  for (int i = 0; i < _swapchainImageViews.size(); i++) {
    vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
  }
}

void VulkanRendererBackend::shutdown() {
  DECLARE_LOG_CATEGORY(VULKAN_SHUTDOWN); // Dedicated category for cleanup
  vkDeviceWaitIdle(_device);
  OE_LOG(VULKAN_BACKEND, INFO, "Shutting down vulkan renderer");
  OE_LOG(VULKAN_SHUTDOWN, INFO, "Clearing metalRoughMaterial resources")
  metalRoughMaterial.clear_resources(_device);

  OE_LOG(VULKAN_SHUTDOWN, INFO, "Destroying sync strucutures");
  for (int i = 0; i < FRAME_OVERLAP; i++) {
    vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);
    // destroy sync objects
    vkDestroyFence(_device, _frames[i]._renderFence, nullptr);
    vkDestroySemaphore(_device, _frames[i]._renderSemaphore, nullptr);
    vkDestroySemaphore(_device, _frames[i]._swapchainSemaphore, nullptr);
    // Should handle anything elese
    _frames[i]._deletionQueue.flush();
  }
  OE_LOG(VULKAN_SHUTDOWN, INFO, "Destroying global allocator");
  globalDescriptorAllocator.destroy_pools(_device);
  OE_LOG(VULKAN_SHUTDOWN, INFO, "Flushing deletion queue");
  _mainDeletionQueue.flush();
  OE_LOG(VULKAN_SHUTDOWN, INFO, "Destroying swapchain");
  destroy_swapchain();
  OE_LOG(VULKAN_SHUTDOWN, INFO, "Destroying surface")
  vkDestroySurfaceKHR(_instance, _surface, nullptr);
  OE_LOG(VULKAN_SHUTDOWN, INFO, "Destroying device")
  vkDestroyDevice(_device, nullptr);
  vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
  vkDestroyInstance(_instance, nullptr);
}

void VulkanRendererBackend::init_sync_structures() {
  // create syncronization structures
  // one fence to control when the gpu has finished rendering the frame,
  // and 2 semaphores to syncronize rendering with swapchain
  // we want the fence to start signalled so we can wait on it on the first
  // frame
  VkFenceCreateInfo fenceCreateInfo =
      vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
  VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info(0);

  for (int i = 0; i < FRAME_OVERLAP; i++) {
    VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr,
                           &_frames[i]._renderFence));

    VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr,
                               &_frames[i]._swapchainSemaphore));
    VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr,
                               &_frames[i]._renderSemaphore));
  }
  // Immediate fence
  VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_immFence));

  OE_LOG(VULKAN_BACKEND, INFO, "Sync structures created");

  _mainDeletionQueue.push_function(
      [this]() { vkDestroyFence(_device, _immFence, nullptr); });
}

void VulkanRendererBackend::init_descriptors() {
  // create a descriptor pool that will hold 10 sets with 1 image each
  std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> sizes = {
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}};

  globalDescriptorAllocator.init(_device, 10, sizes);

  // make the descriptor set layout for our compute draw
  {
    DescriptorLayoutBuilder builder;
    builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    _drawImageDescriptorLayout =
        builder.build(_device, VK_SHADER_STAGE_COMPUTE_BIT);
  }

  // make the global descriptor set layout for the gemoetry render pass
  {
    DescriptorLayoutBuilder builder;
    builder.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    _gpuSceneDataDescriptorLayout = builder.build(
        _device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
  }

  // Allocate the texture DescriptorLayout
  {
    DescriptorLayoutBuilder builder;
    builder.add_binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    _singleImageDescriptorLayout =
        builder.build(_device, VK_SHADER_STAGE_FRAGMENT_BIT);
  }

  // allocate a descriptor set for our draw image
  _drawImageDescriptors =
      globalDescriptorAllocator.allocate(_device, _drawImageDescriptorLayout);

  DescriptorWriter writer;
  writer.write_image(0, _drawImage.imageView, VK_NULL_HANDLE,
                     VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

  writer.update_set(_device, _drawImageDescriptors);
  // make sure both the descriptor allocator and the new layout get cleaned up
  // properly

  for (int i = 0; i < FRAME_OVERLAP; i++) {
    // create a descriptor pool
    std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> frame_sizes = {
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4},
    };

    _frames[i]._frameDescriptors = DescriptorAllocatorGrowable{};
    _frames[i]._frameDescriptors.init(_device, 1000, frame_sizes);

    _mainDeletionQueue.push_function(
        [&, i]() { _frames[i]._frameDescriptors.destroy_pools(_device); });
  }

  OE_LOG(VULKAN_BACKEND, INFO, "Descriptor Layouts allocated");

  _mainDeletionQueue.push_function([&]() {
    vkDestroyDescriptorSetLayout(_device, _singleImageDescriptorLayout,
                                 nullptr);
  });

  _mainDeletionQueue.push_function([&]() {
    vkDestroyDescriptorSetLayout(_device, _gpuSceneDataDescriptorLayout,
                                 nullptr);
  });

  _mainDeletionQueue.push_function([&]() {
    vkDestroyDescriptorSetLayout(_device, _drawImageDescriptorLayout, nullptr);
  });
}

/**
 * @brief Initializes pipes for rendering
 * @detail Currently initializes all pipelines used due to low overhead
 */
void VulkanRendererBackend::init_pipelines() {
  // TODO: Configurable/runtime decision on what pipelines are compiled at
  // source
  init_mesh_pipeline();
  metalRoughMaterial.build_pipelines(this);
}

void VulkanRendererBackend::init_mesh_pipeline() {
  VkShaderModule triangleFragShader;
  if (!vkutil::load_shader_module("../../shaders/tex_image.frag.spv", _device,
                                  &triangleFragShader)) {
    OE_LOG(VULKAN_BACKEND, INFO, "Error building default mesh fragment shader");
  } else {
    OE_LOG(VULKAN_BACKEND, INFO,
           "Default mesh fragment shader successfully loaded");
  }

  VkShaderModule triangleVertexShader;
  if (!vkutil::load_shader_module(
          "../../shaders/colored_triangle_mesh.vert.spv", _device,
          &triangleVertexShader)) {
    OE_LOG(VULKAN_BACKEND, INFO, "Error building default mesh vertext shader");
  } else {
    OE_LOG(VULKAN_BACKEND, INFO,
           "Default mesh vertex shader successfully loaded");
  }

  VkPushConstantRange bufferRange{};
  bufferRange.offset = 0;
  bufferRange.size = sizeof(GPUDrawPushConstants);
  bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkPipelineLayoutCreateInfo pipeline_layout_info =
      vkinit::pipeline_layout_create_info();
  pipeline_layout_info.pPushConstantRanges = &bufferRange;
  pipeline_layout_info.pushConstantRangeCount = 1;
  pipeline_layout_info.pSetLayouts = &_singleImageDescriptorLayout;
  pipeline_layout_info.setLayoutCount = 1;
  VK_CHECK(vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr,
                                  &_meshPipelineLayout));

  PipelineBuilder pipelineBuilder;

  // use the triangle layout we created
  pipelineBuilder._pipelineLayout = _meshPipelineLayout;
  // connecting the vertex and pixel shaders to the pipeline
  pipelineBuilder.set_shaders(triangleVertexShader, triangleFragShader);
  // it will draw triangles
  pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  // filled triangles
  pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
  // no backface culling
  pipelineBuilder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
  // no multisampling
  pipelineBuilder.set_multisampling_none();
  //  blending
  // pipelineBuilder.disable_blending();
  pipelineBuilder.enable_blending_additive();
  // pipelineBuilder.enable_blending_alphablend();

  // pipelineBuilder.disable_depthtest();
  pipelineBuilder.enable_depthtest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);

  // connect the image format we will draw into, from draw image
  pipelineBuilder.set_color_attachment_format(_drawImage.imageFormat);
  pipelineBuilder.set_depth_format(
      _depthImage.imageFormat); // finally build the pipeline

  _meshPipeline = pipelineBuilder.build_pipeline(_device);

  // clean structures
  vkDestroyShaderModule(_device, triangleFragShader, nullptr);
  vkDestroyShaderModule(_device, triangleVertexShader, nullptr);

  _mainDeletionQueue.push_function([&]() {
    vkDestroyPipelineLayout(_device, _meshPipelineLayout, nullptr);
    vkDestroyPipeline(_device, _meshPipeline, nullptr);
  });
}

void VulkanRendererBackend::destroy_buffer(const AllocatedBuffer &buffer) {
  vmaDestroyBuffer(_allocator, buffer.buffer, buffer.allocation);
}

AllocatedBuffer
VulkanRendererBackend::create_buffer(size_t allocSize, VkBufferUsageFlags usage,
                                     VmaMemoryUsage memoryUsage) {
  // Allocate buffer
  VkBufferCreateInfo bufferInfo = {.sType =
                                       VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bufferInfo.pNext = nullptr;
  bufferInfo.size = allocSize;
  bufferInfo.usage = usage;

  VmaAllocationCreateInfo vmaallocInfo = {};
  vmaallocInfo.usage = memoryUsage;
  vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
  AllocatedBuffer newBuffer;

  // allocate the buffer
  VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo,
                           &newBuffer.buffer, &newBuffer.allocation,
                           &newBuffer.info));

  return newBuffer;
}

void VulkanRendererBackend::init_default_data() {
  // 3 default textures, white, grey, black. 1 pixel each
  uint32_t white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
  _whiteImage =
      create_image((void *)&white, VkExtent3D{1, 1, 1},
                   VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

  uint32_t grey = glm::packUnorm4x8(glm::vec4(0.66f, 0.66f, 0.66f, 1));
  _greyImage =
      create_image((void *)&grey, VkExtent3D{1, 1, 1}, VK_FORMAT_R8G8B8A8_UNORM,
                   VK_IMAGE_USAGE_SAMPLED_BIT);

  uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 1));
  _blackImage =
      create_image((void *)&black, VkExtent3D{1, 1, 1},
                   VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

  // checkerboard image
  uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));

  std::array<uint32_t, 16 * 16> pixels; // for 16x16 checkerboard texture
  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 16; y++) {
      pixels[y * 16 + x] = ((x % 2) ^ (y % 2)) ? black : grey;
    }
  }
  _errorCheckerboardImage =
      create_image(pixels.data(), VkExtent3D{16, 16, 1},
                   VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
  OE_LOG(VULKAN_BACKEND, INFO, "Fallback checkerboard image created");

  VkSamplerCreateInfo sampl = {.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};

  sampl.magFilter = VK_FILTER_NEAREST;
  sampl.minFilter = VK_FILTER_NEAREST;

  vkCreateSampler(_device, &sampl, nullptr, &_defaultSamplerNearest);

  sampl.magFilter = VK_FILTER_LINEAR;
  sampl.minFilter = VK_FILTER_LINEAR;
  vkCreateSampler(_device, &sampl, nullptr, &_defaultSamplerLinear);

  // Default GLTF textures
  GLTFMetallic_Roughness::MaterialResources materialResources;
  // default the material textures
  materialResources.colorImage = _whiteImage;
  materialResources.colorSampler = _defaultSamplerLinear;
  materialResources.metalRoughImage = _whiteImage;
  materialResources.metalRoughSampler = _defaultSamplerLinear;
  // set the uniform buffer for the material data
  AllocatedBuffer materialConstants = create_buffer(
      sizeof(GLTFMetallic_Roughness::MaterialConstants),
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

  // write the buffer
  GLTFMetallic_Roughness::MaterialConstants *sceneUniformData =
      (GLTFMetallic_Roughness::MaterialConstants *)
          materialConstants.allocation->GetMappedData();
  sceneUniformData->colorFactors = glm::vec4{1, 1, 1, 1};
  sceneUniformData->metal_rough_factors = glm::vec4{1, 0.5, 0, 0};

  _mainDeletionQueue.push_function(
      [=, this]() { destroy_buffer(materialConstants); });

  materialResources.dataBuffer = materialConstants.buffer;
  materialResources.dataBufferOffset = 0;

  defaultData = metalRoughMaterial.write_material(
      _device, MaterialPass::MainColor, materialResources,
      globalDescriptorAllocator);

  _mainDeletionQueue.push_function([&]() {
    vkDestroySampler(_device, _defaultSamplerNearest, nullptr);
    vkDestroySampler(_device, _defaultSamplerLinear, nullptr);

    destroy_image(_whiteImage);
    destroy_image(_greyImage);
    destroy_image(_blackImage);
    destroy_image(_errorCheckerboardImage);
  });
}

AllocatedImage VulkanRendererBackend::create_image(void *data, VkExtent3D size,
                                                   VkFormat format,
                                                   VkImageUsageFlags usage,
                                                   bool mipmapped) {
  size_t data_size = size.depth * size.width * size.height * 4;
  AllocatedBuffer uploadbuffer = create_buffer(
      data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

  memcpy(uploadbuffer.info.pMappedData, data, data_size);

  AllocatedImage new_image = create_image(
      size, format,
      usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
      mipmapped);

  immediate_submit([&](VkCommandBuffer cmd) {
    vkutil::transition_image(cmd, new_image.image, VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy copyRegion = {};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;

    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel = 0;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = 1;
    copyRegion.imageExtent = size;

    // copy the buffer into the image
    vkCmdCopyBufferToImage(cmd, uploadbuffer.buffer, new_image.image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &copyRegion);

    vkutil::transition_image(cmd, new_image.image,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  });

  destroy_buffer(uploadbuffer);

  return new_image;
}

AllocatedImage VulkanRendererBackend::create_image(VkExtent3D size,
                                                   VkFormat format,
                                                   VkImageUsageFlags usage,
                                                   bool mipmapped) {
  AllocatedImage newImage;
  newImage.imageFormat = format;
  newImage.imageExtent = size;

  VkImageCreateInfo img_info = vkinit::image_create_info(format, usage, size);
  if (mipmapped) {
    img_info.mipLevels = static_cast<uint32_t>(std::floor(
                             std::log2(std::max(size.width, size.height)))) +
                         1;
  }

  // always allocate images on dedicated GPU memory
  VmaAllocationCreateInfo allocinfo = {};
  allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  allocinfo.requiredFlags =
      VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  // allocate and create the image
  VK_CHECK(vmaCreateImage(_allocator, &img_info, &allocinfo, &newImage.image,
                          &newImage.allocation, nullptr));

  // if the format is a depth format, we will need to have it use the correct
  // aspect flag
  VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
  if (format == VK_FORMAT_D32_SFLOAT) {
    aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
  }

  // build a image-view for the image
  VkImageViewCreateInfo view_info =
      vkinit::imageview_create_info(format, newImage.image, aspectFlag);
  view_info.subresourceRange.levelCount = img_info.mipLevels;
  VK_CHECK(
      vkCreateImageView(_device, &view_info, nullptr, &newImage.imageView));

  return newImage;
}

/**
 * @brief Immediate draw submit, such as imgui rendering
 * @param function The function to be used in immediate submit
 */
void VulkanRendererBackend::immediate_submit(
    std::function<void(VkCommandBuffer cmd)> &&function) {
  VK_CHECK(vkResetFences(_device, 1, &_immFence));
  VK_CHECK(vkResetCommandBuffer(_immCommandBuffer, 0));

  VkCommandBuffer cmd = _immCommandBuffer;

  VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

  function(cmd);

  VK_CHECK(vkEndCommandBuffer(cmd));

  VkCommandBufferSubmitInfo cmdinfo = vkinit::command_buffer_submit_info(cmd);
  VkSubmitInfo2 submit = vkinit::submit_info(&cmdinfo, nullptr, nullptr);

  // submit command buffer to the queue and execute it.
  //  _renderFence will now block until the graphic commands finish execution
  VK_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submit, _immFence));

  VK_CHECK(vkWaitForFences(_device, 1, &_immFence, true, 9999999999));
}

/**
 * @brief Destroys provided image and associated image view
 * @param img AllocatedImage to be destroyed
 */
void VulkanRendererBackend::destroy_image(const AllocatedImage &img) {
  vkDestroyImageView(_device, img.imageView, nullptr);
  vmaDestroyImage(_allocator, img.image, img.allocation);
}

/**
 * @brief Binds a provided material information to the appropriate GPU resources
 * for use
 * @param device
 * @param pass
 * @param resources
 * @param descriptorAllocator
 * @return
 */
MaterialInstance GLTFMetallic_Roughness::write_material(
    VkDevice device, MaterialPass pass, const MaterialResources &resources,
    DescriptorAllocatorGrowable &descriptorAllocator) {
  MaterialInstance matData;
  matData.passType = pass;
  if (pass == MaterialPass::Transparent) {
    matData.pipeline = &transparentPipeline;
  } else {
    matData.pipeline = &opaquePipeline;
  }

  OE_LOG(VULKAN_BACKEND, INFO, "Just before the allocate call");
  matData.materialSet = descriptorAllocator.allocate(device, materialLayout);
  OE_LOG(VULKAN_BACKEND, INFO, "Just after the allocate call");
  writer.clear();
  writer.write_buffer(0, resources.dataBuffer, sizeof(MaterialConstants),
                      resources.dataBufferOffset,
                      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  writer.write_image(1, resources.colorImage.imageView, resources.colorSampler,
                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  writer.write_image(2, resources.metalRoughImage.imageView,
                     resources.metalRoughSampler,
                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

  writer.update_set(device, matData.materialSet);

  return matData;
}

void GLTFMetallic_Roughness::build_pipelines(VulkanRendererBackend *engine) {
  VkShaderModule meshFragShader;
  if (!vkutil::load_shader_module("../../shaders/mesh.frag.glsl.spv",
                                  engine->_device, &meshFragShader)) {
    fmt::println("Error when building the triangle fragment shader module");
  }

  VkShaderModule meshVertexShader;
  if (!vkutil::load_shader_module("../../shaders/mesh.vert.glsl.spv",
                                  engine->_device, &meshVertexShader)) {
    fmt::println("Error when building the triangle vertex shader module");
  }

  VkPushConstantRange matrixRange{};
  matrixRange.offset = 0;
  matrixRange.size = sizeof(GPUDrawPushConstants);
  matrixRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  DescriptorLayoutBuilder layoutBuilder;
  layoutBuilder.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  layoutBuilder.add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  layoutBuilder.add_binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

  materialLayout =
      layoutBuilder.build(engine->_device, VK_SHADER_STAGE_VERTEX_BIT |
                                               VK_SHADER_STAGE_FRAGMENT_BIT);
  VkDescriptorSetLayout layouts[] = {engine->_gpuSceneDataDescriptorLayout,
                                     materialLayout};

  VkPipelineLayoutCreateInfo mesh_layout_info =
      vkinit::pipeline_layout_create_info();
  mesh_layout_info.setLayoutCount = 2;
  mesh_layout_info.pSetLayouts = layouts;
  mesh_layout_info.pPushConstantRanges = &matrixRange;
  mesh_layout_info.pushConstantRangeCount = 1;

  VkPipelineLayout newLayout;
  VK_CHECK(vkCreatePipelineLayout(engine->_device, &mesh_layout_info, nullptr,
                                  &newLayout));

  opaquePipeline.layout = newLayout;
  transparentPipeline.layout = newLayout;

  // build the stage-create-info for both vertex and fragment stages. This lets
  // the pipeline know the shader modules per stage
  PipelineBuilder pipelineBuilder;
  pipelineBuilder.set_shaders(meshVertexShader, meshFragShader);
  pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
  pipelineBuilder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
  pipelineBuilder.set_multisampling_none();
  pipelineBuilder.disable_blending();
  pipelineBuilder.enable_depthtest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);

  // render format
  pipelineBuilder.set_color_attachment_format(engine->_drawImage.imageFormat);
  pipelineBuilder.set_depth_format(engine->_depthImage.imageFormat);

  // use the triangle layout we created
  pipelineBuilder._pipelineLayout = newLayout;

  // finally build the pipeline
  opaquePipeline.pipeline = pipelineBuilder.build_pipeline(engine->_device);

  // create the transparent variant
  pipelineBuilder.enable_blending_additive();

  pipelineBuilder.enable_depthtest(false, VK_COMPARE_OP_GREATER_OR_EQUAL);

  transparentPipeline.pipeline =
      pipelineBuilder.build_pipeline(engine->_device);

  vkDestroyShaderModule(engine->_device, meshFragShader, nullptr);
  vkDestroyShaderModule(engine->_device, meshVertexShader, nullptr);
}

void GLTFMetallic_Roughness::clear_resources(VkDevice device) {
  // They share a layout, only delete one layout
  vkDestroyPipelineLayout(device, opaquePipeline.layout, nullptr);
  vkDestroyPipeline(device, opaquePipeline.pipeline, nullptr);
  vkDestroyPipeline(device, transparentPipeline.pipeline, nullptr);
  vkDestroyDescriptorSetLayout(device, materialLayout, nullptr);
}
