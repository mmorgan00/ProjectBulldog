#pragma once

#include "../../../util/containers.hpp"
#include "vk_mem_alloc.h"
#include "vulkan_descriptors.hpp"
#include "vulkan_initializers.hpp"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

#include "../../renderer_backend.hpp"
#include "vulkan_types.hpp"

constexpr unsigned int FRAME_OVERLAP = 2;
// Forward declare so helper structs can take in pointers
class VulkanRendererBackend;

struct FrameData {
  VkSemaphore _swapchainSemaphore, _renderSemaphore;
  VkFence _renderFence;

  VkCommandPool _commandPool;
  VkCommandBuffer _mainCommandBuffer;

  DeletionQueue _deletionQueue;
  DescriptorAllocatorGrowable _frameDescriptors;
};

struct GLTFMetallic_Roughness {
	MaterialPipeline opaquePipeline;
	MaterialPipeline transparentPipeline;

	VkDescriptorSetLayout materialLayout;

	struct MaterialConstants {
		glm::vec4 colorFactors;
		glm::vec4 metal_rough_factors;
		//padding, we need it anyway for uniform buffers
		glm::vec4 extra[14];
	};

	struct MaterialResources {
		AllocatedImage colorImage;
		VkSampler colorSampler;
		AllocatedImage metalRoughImage;
		VkSampler metalRoughSampler;
		VkBuffer dataBuffer;
		uint32_t dataBufferOffset;
	};

	DescriptorWriter writer;

	void build_pipelines(VulkanRendererBackend* engine);
	void clear_resources(VkDevice device);

	MaterialInstance write_material(VkDevice device, MaterialPass pass, const MaterialResources& resources, DescriptorAllocatorGrowable& descriptorAllocator);
};




class VulkanRendererBackend : public RendererBackend {
public:
  bool init(app_state &state) override;
  void shutdown() override;

  int _frameNumber{0};
  DeletionQueue _mainDeletionQueue;
  GPUSceneData sceneData;

  VkDevice _device;                          // Vulkan device for commands
  // Public so that Material builders can access/write
  VkDescriptorSetLayout _gpuSceneDataDescriptorLayout;
  VkDescriptorSetLayout _singleImageDescriptorLayout;
  // Draw resources
  AllocatedImage _drawImage;
  AllocatedImage _depthImage;
  VkExtent2D _drawExtent;
  float renderScale = 1.f;



private:
  // Initializes Vulkan instance, devices, and queues
  void init_inst(app_state &state);
  // Initializes starting swapchain
  void init_swapchain();
  // Inits command pools and buffers
  void init_commands();
  // Initializes fences and semaphores for each frame
  void init_sync_structures();
  // Initializes descriptor sets for shaders
  void init_descriptors();
  //
  void init_pipelines();
  // Pipeline create functions
  // TODO: Hardcoding each pipeline create function will not scale
  void init_mesh_pipeline();

  void init_default_data();

  void run() override;
  void draw_geometry(VkCommandBuffer cmd);

  void create_swapchain(uint32_t width, uint32_t height);
  void destroy_swapchain();


  //> Default data
  AllocatedImage _whiteImage;
  AllocatedImage _blackImage;
  AllocatedImage _greyImage;
  AllocatedImage _errorCheckerboardImage;
  VkSampler _defaultSamplerLinear;
  VkSampler _defaultSamplerNearest;
  MaterialInstance defaultData;
  GLTFMetallic_Roughness metalRoughMaterial;
  //< Default data


  void immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function);

  //> Buffer management
  AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage,
                                VmaMemoryUsage memoryUsage);
  void destroy_buffer(const AllocatedBuffer &buffer);
  //< Buffer management

  //< Image management
  AllocatedImage create_image(VkExtent3D size, VkFormat format,
                              VkImageUsageFlags usage, bool mipmapped = false);
  AllocatedImage create_image(void *data, VkExtent3D size, VkFormat format,
                              VkImageUsageFlags usages, bool mipmapped = false);
  void destroy_image(const AllocatedImage &img);
  //> Image management

  //< Image management
  DescriptorAllocatorGrowable globalDescriptorAllocator;

  // Window resources
  VkExtent2D _windowExtent{1700, 900};
  // Vulkan resources
  VkInstance _instance;                      // Vulkan library handle
  VkDebugUtilsMessengerEXT _debug_messenger; // Vulkan debug output handle
  VkSurfaceKHR _surface;                     // Vulkan window surface
  VkPhysicalDevice _chosenGPU;               // GPU chosen as the default device
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

  //< mesh pipeline
  VkPipelineLayout _meshPipelineLayout;
  VkPipeline _meshPipeline;
  //> mesh pipeline
  // Window
  struct SDL_Window *_window{nullptr};

  FrameData &get_current_frame() {
    return _frames[_frameNumber % FRAME_OVERLAP];
  };
};
