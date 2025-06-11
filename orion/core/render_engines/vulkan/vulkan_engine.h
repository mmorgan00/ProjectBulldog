// Copyright 2025 Max Morgan

#ifndef ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_ENGINE_H_
#define ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_ENGINE_H_
#include <core/render_engine.h>

#include <vector>

#include "core/engine_types.h"
#include "core/render_engines/vulkan/vulkan_types.h"
#include "util/logger.h"

DECLARE_LOG_CATEGORY(VULKAN_ENGINE);
class VulkanEngine : public RenderEngine {
  bool _isInitialized{false};
  int _frameNumber{0};
  bool stop_rendering{false};
  VkExtent2D _windowExtent{1700, 900};

  struct SDL_Window* _window{nullptr};

  //< Vk resource handles
  VkInstance _instance;                       // Vulkan library handle
  VkDebugUtilsMessengerEXT _debug_messenger;  // Vulkan debug output handle
  VkPhysicalDevice _chosenGPU;  // GPU chosen as the default device
  VkDevice _device;             // Vulkan device for commands
  VkSurfaceKHR _surface;        // Vulkan window surface
                                // > Vk Resource handles
  FrameData _frames[FRAME_OVERLAP];

  FrameData& get_current_frame() {
    return _frames[_frameNumber % FRAME_OVERLAP];
  }

  VkQueue _graphicsQueue;
  uint32_t _graphicsQueueFamily;

  void init_vulkan(app_state& state);
  void init_swapchain();
  void init_commands();
  void init_sync_structures();

  VkSwapchainKHR _swapchain;
  VkFormat _swapchainImageFormat;

  void create_swapchain(uint32_t width, uint32_t height);
  void destroy_swapchain();

  std::vector<VkImage> _swapchainImages;
  std::vector<VkImageView> _swapchainImageViews;
  VkExtent2D _swapchainExtent;

 public:
  bool init(app_state& state) override;
  // draw loop
  void draw() override;
  // shuts down the engine
  void cleanup() override;
};
#endif  // ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_ENGINE_H_
