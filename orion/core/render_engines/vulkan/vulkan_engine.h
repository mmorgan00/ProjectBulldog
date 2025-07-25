// Copyright 2025 Max Morgan

#ifndef ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_ENGINE_H_
#define ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_ENGINE_H_
#include <core/render_engine.h>
#include <vulkan/vulkan_core.h>

#include <vector>

#include "core/engine_types.h"
#include "core/render_engines/vulkan/vulkan_descriptors.h"
#include "core/render_engines/vulkan/vulkan_types.h"
#include "util/logger.h"

DECLARE_LOG_CATEGORY(VULKAN_ENGINE);
class VulkanEngine : public RenderEngine {
  bool _isInitialized{false};
  int _frameNumber{0};
  bool stop_rendering{false};
  VkExtent2D _windowExtent{1700, 900};
  // To select the correct sync and command objects, we need to keep track of
  // the current frame and (swapchain) image index
  uint32_t currentFrame{0};
  uint32_t currentSemaphore{0};

  struct SDL_Window* _window{nullptr};

  //< Object managers
  DeletionQueue _mainDeletionQueue;
  VmaAllocator _allocator;
  //> Object managers

  //< Render loop resources
  AllocatedImage _drawImage;
  VkExtent2D _drawExtent;
  VkExtent2D _drawImageExtent;
  //> Render loop resources

  //< Vk resource handles
  VkInstance _instance;                       // Vulkan library handle
  VkDebugUtilsMessengerEXT _debug_messenger;  // Vulkan debug output handle
  VkPhysicalDevice _chosenGPU;  // GPU chosen as the default device
  VkDevice _device;             // Vulkan device for commands
  VkSurfaceKHR _surface;        // Vulkan window surface
                                // > Vk Resource handles
  FrameData _frames[MAX_CONCURRENT_FRAMES];
  FrameData& get_current_frame() {
    return _frames[_frameNumber % MAX_CONCURRENT_FRAMES];
  }
  // Semaphores are used to coordinate operations within the graphics queue and
  // ensure correct command ordering
  std::vector<VkSemaphore> presentCompleteSemaphores{};
  std::vector<VkSemaphore> renderCompleteSemaphores{};
  // Fences are used to make sure command buffers aren't rerecorded until
  // they've finished executing
  std::array<VkFence, MAX_CONCURRENT_FRAMES> waitFences{};

  VkQueue _graphicsQueue;
  uint32_t _graphicsQueueFamily;

  void init_vulkan(app_state& state);
  void init_swapchain();
  void init_commands();
  void init_sync_structures();
  void init_descriptors();
  void init_pipelines();
  void init_background_pipeline();
  void init_default_pipeline();

  VkSwapchainKHR _swapchain;
  VkFormat _swapchainImageFormat;

  void create_swapchain(uint32_t width, uint32_t height);
  void destroy_swapchain();

  std::vector<VkImage> _swapchainImages;
  std::vector<VkImageView> _swapchainImageViews;
  VkExtent2D _swapchainExtent;

 public:
  // Descriptor sets
  DescriptorAllocator globalDescriptorAllocator;

  VkDescriptorSet _drawImageDescriptors;
  VkDescriptorSetLayout _drawImageDescriptorLayout;

  // Pipelines
  VkPipeline _gradientPipeline;
  VkPipelineLayout _gradientPipelineLayout;

  VkPipelineLayout _defaultPipelineLayout;
  VkPipeline _defaultPipeline;

  bool init(app_state& state) override;
  void loadScene() override;
  // draw loop
  void draw() override;
  void draw_background(VkCommandBuffer cmd);
  void draw_geometry(VkCommandBuffer cmd);
  // shuts down the engine
  void cleanup() override;
};
#endif  // ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_ENGINE_H_
