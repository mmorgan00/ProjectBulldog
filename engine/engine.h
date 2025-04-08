// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <engine_types.h>
#include <vk_descriptors.h>

#include <vector>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"
#include "vk_mem_alloc.h"

struct ComputePushConstants {
  glm::vec4 data1;
  glm::vec4 data2;
  glm::vec4 data3;
  glm::vec4 data4;
};

struct ComputeEffect {
  const char* name;

  VkPipeline pipeline;
  VkPipelineLayout layout;

  ComputePushConstants data;
};

//> framedata
struct FrameData {
  VkSemaphore _swapchainSemaphore, _renderSemaphore;
  VkFence _renderFence;

  VkCommandPool _commandPool;
  VkCommandBuffer _mainCommandBuffer;

  DeletionQueue _deletionQueue;
};

constexpr unsigned int FRAME_OVERLAP = 2;
//< framedata

class VulkanEngine {
 public:
  bool _isInitialized{false};
  int _frameNumber{0};
  DeletionQueue _mainDeletionQueue;
  VmaAllocator _allocator;

  std::vector<ComputeEffect> backgroundEffects;
  int currentBackgroundEffect{0};

  // draw resources
  AllocatedImage _drawImage;
  AllocatedImage _depthImage;
  VkExtent2D _drawExtent;

  VkExtent2D _windowExtent{1700, 900};

  struct SDL_Window* _window{nullptr};

  //> inst_init
  VkInstance _instance;                       // Vulkan library handle
  VkDebugUtilsMessengerEXT _debug_messenger;  // Vulkan debug output handle
  VkPhysicalDevice _chosenGPU;  // GPU chosen as the default device
  VkDevice _device;             // Vulkan device for commands
  VkSurfaceKHR _surface;        // Vulkan window surface
                                //< inst_init

  DescriptorAllocator globalDescriptorAllocator;

  VkDescriptorSet _drawImageDescriptors;
  VkDescriptorSetLayout _drawImageDescriptorLayout;

  VkPipeline _gradientPipeline;
  VkPipelineLayout _gradientPipelineLayout;

  // Immediates
  VkFence _immFence;
  VkCommandBuffer _immCommandBuffer;
  VkCommandPool _immCommandPool;

  //> queues
  FrameData _frames[FRAME_OVERLAP];

  FrameData& get_current_frame() {
    return _frames[_frameNumber % FRAME_OVERLAP];
  };

  VkQueue _graphicsQueue;
  uint32_t _graphicsQueueFamily;
  //< queues

  //> swap_init
  VkSwapchainKHR _swapchain;
  VkFormat _swapchainImageFormat;

  std::vector<VkImage> _swapchainImages;
  std::vector<VkImageView> _swapchainImageViews;
  VkExtent2D _swapchainExtent;
  //< swap_init

  VkPipelineLayout _trianglePipelineLayout;
  VkPipeline _trianglePipeline;

  //< mesh pipeline
  VkPipelineLayout _meshPipelineLayout;
  VkPipeline _meshPipeline;

  GPUMeshBuffers rectangle;
  std::vector<std::shared_ptr<MeshAsset>> testMeshes;

  void init_mesh_pipeline();
  //<mesh pipeline

  // initializes everything in the engine
  void init();

  // shuts down the engine
  void cleanup();

  // draw loop
  void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);
  void draw();
  void draw_background(VkCommandBuffer cmd);

  GPUMeshBuffers uploadMesh(std::span<uint32_t> indices,
                            std::span<Vertex> vertices);

  // run main loop
  void run();

  bool stop_rendering{false};

 private:
  void init_vulkan();
  void init_swapchain();
  void create_swapchain(uint32_t width, uint32_t height);
  void destroy_swapchain();
  void init_commands();
  void init_sync_structures();
  void init_descriptors();
  void init_pipelines();
  void init_background_pipelines();
  void init_imgui();
  void init_triangle_pipeline();
  void draw_geometry(VkCommandBuffer cmd);
  void draw_imgui(VkCommandBuffer cmd, VkImageView targetImageView);
  AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage,
                                VmaMemoryUsage memoryUsage);
  void destroy_buffer(const AllocatedBuffer& buffer);
  void init_default_data();
};
