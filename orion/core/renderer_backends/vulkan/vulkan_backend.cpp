#include "vulkan_backend.hpp"

#include "VkBootstrap.h"
#include "core/engine_types.hpp"
#include "util/logger.hpp"
#include "vulkan_images.hpp"
#include "vulkan_pipelines.hpp"
#include <SDL.h>
#include <SDL_vulkan.h>
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
  // draw_geometry(cmd);

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

bool VulkanRendererBackend::init(app_state &state) {
  init_inst(state);
  init_swapchain();
  init_commands();
  init_sync_structures();
  init_descriptors(); // not rendering anything however. 
  init_pipelines();
  // TODO:
  // init_imgui();
  // init_default_data();
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

void VulkanRendererBackend::destroy_swapchain(){
	vkDestroySwapchainKHR(_device, _swapchain, nullptr);

	// destroy swapchain resources
	for (int i = 0; i < _swapchainImageViews.size(); i++) {
		vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
	}
}

void VulkanRendererBackend::shutdown() {
  vkDeviceWaitIdle(_device);

  OE_LOG(VULKAN_BACKEND, INFO, "Shutting down vulkan renderer");

  for (int i = 0; i < FRAME_OVERLAP; i++) {
    vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);

    // destroy sync objects
    vkDestroyFence(_device, _frames[i]._renderFence, nullptr);
    vkDestroySemaphore(_device, _frames[i]._renderSemaphore, nullptr);
    vkDestroySemaphore(_device, _frames[i]._swapchainSemaphore, nullptr);

    _frames[i]._deletionQueue.flush();
  }
  globalDescriptorAllocator.clear_pools(_device);
  _mainDeletionQueue.flush();
  destroy_swapchain();

  vkDestroySurfaceKHR(_instance, _surface, nullptr);

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
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}};

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
    vkDestroyDescriptorSetLayout(_device, _singleImageDescriptorLayout, nullptr);
  });

  _mainDeletionQueue.push_function([&]() {
    vkDestroyDescriptorSetLayout(_device, _gpuSceneDataDescriptorLayout, nullptr);
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
