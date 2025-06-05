#pragma once

#include "../../../util/containers.hpp"
#include <cstdint>
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include "vulkan_initializers.hpp"
#include "vulkan_descriptors.hpp"
#include <vector>

#include "vulkan_types.hpp"
#include "../../renderer_backend.hpp"



constexpr unsigned int FRAME_OVERLAP = 2;

struct FrameData {
  VkSemaphore _swapchainSemaphore, _renderSemaphore;
  VkFence _renderFence;

  VkCommandPool _commandPool;
  VkCommandBuffer _mainCommandBuffer;

  DeletionQueue _deletionQueue;
  DescriptorAllocatorGrowable _frameDescriptors;
};


class VulkanRendererBackend : public RendererBackend {
public:
  bool init(app_state& state) override;
  void shutdown() override;


private:
  // Initializes Vulkan instance, devices, and queues
  void init_inst(app_state& state);
  // Initializes starting swapchain
  void init_swapchain();
  // Inits command pools and buffers
  void init_commands();
  // Initializes fences and semaphores for each frame
  void init_sync_structures();

  void create_swapchain(uint32_t width, uint32_t height);

  DeletionQueue _mainDeletionQueue;

  // Window resources
  VkExtent2D _windowExtent{1700, 900};
    // Vulkan resources
  VkInstance _instance;                       // Vulkan library handle
  VkDebugUtilsMessengerEXT _debug_messenger;  // Vulkan debug output handle
  VkSurfaceKHR _surface;        // Vulkan window surface
  VkPhysicalDevice _chosenGPU;  // GPU chosen as the default device
  VkDevice _device;             // Vulkan device for commands
  VmaAllocator _allocator;
  // Queues
  VkQueue _graphicsQueue;
  uint32_t _graphicsQueueFamily;

  // Swapchain resources
  VkSwapchainKHR _swapchain;
  VkFormat _swapchainImageFormat;

  std::vector<VkImage> _swapchainImages;
  std::vector<VkImageView> _swapchainImageViews;
  VkExtent2D _swapchainExtent;

  VkDescriptorSet _drawImageDescriptors;
  VkDescriptorSetLayout _drawImageDescriptorLayout;

  
  FrameData _frames[FRAME_OVERLAP];

  // Immediates
  VkFence _immFence;
  VkCommandBuffer _immCommandBuffer;
  VkCommandPool _immCommandPool;



  // Draw resources
  AllocatedImage _drawImage;
  AllocatedImage _depthImage;
  VkExtent2D _drawExtent;
  float renderScale = 1.f;



                                              
  // Window
  struct SDL_Window* _window{nullptr};
};


