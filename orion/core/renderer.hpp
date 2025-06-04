#pragma once

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

class Renderer{
public:

void init();
void shutdown();
private:
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

                                              
  // Window
  struct SDL_Window* _window{nullptr};

};
