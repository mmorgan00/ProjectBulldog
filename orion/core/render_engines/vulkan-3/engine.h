
#pragma once

#include <engine_types.h>
#include <vk_descriptors.h>
#include <camera.h>

#include <vector>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"
#include "vk_mem_alloc.h"
#include <vk_loader.h>

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
  DescriptorAllocatorGrowable _frameDescriptors;
};

constexpr unsigned int FRAME_OVERLAP = 2;
//< framedata
// Forward declare to include in struct
class VulkanEngine;

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

	void build_pipelines(VulkanEngine* engine);
	void clear_resources(VkDevice device);

	MaterialInstance write_material(VkDevice device, MaterialPass pass, const MaterialResources& resources, DescriptorAllocatorGrowable& descriptorAllocator);
};

struct RenderObject {
	uint32_t indexCount;
	uint32_t firstIndex;
	VkBuffer indexBuffer;

	MaterialInstance* material;

	glm::mat4 transform;
	VkDeviceAddress vertexBufferAddress;
};

struct DrawContext {
	std::vector<RenderObject> OpaqueSurfaces;
	std::vector<RenderObject> TransparentSurfaces;
};

struct MeshNode : public Node {

	std::shared_ptr<MeshAsset> mesh;

	virtual void Draw(const glm::mat4& topMatrix, DrawContext& ctx) override;
};


class VulkanEngine {
 public:
  bool _isInitialized{false};
  bool resize_requested{false};
  int _frameNumber{0};
  DeletionQueue _mainDeletionQueue;
  VmaAllocator _allocator;

  Camera mainCamera;

  std::vector<ComputeEffect> backgroundEffects;
  int currentBackgroundEffect{0};

  // draw resources
  AllocatedImage _drawImage;
  AllocatedImage _depthImage;
  VkExtent2D _drawExtent;
  float renderScale = 1.f;

  VkExtent2D _windowExtent{1700, 900};

  struct SDL_Window* _window{nullptr};

  GPUSceneData sceneData;
  DrawContext mainDrawContext;
  std::unordered_map<std::string, std::shared_ptr<Node>> loadedNodes; 



  //<Image management
  AllocatedImage create_image(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
  AllocatedImage create_image(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usages, bool mipmapped = false);
  void destroy_image(const AllocatedImage& img);
  //>Image management

  AllocatedImage _whiteImage;
  AllocatedImage _blackImage;
  AllocatedImage _greyImage;
  AllocatedImage _errorCheckerboardImage;

  VkSampler _defaultSamplerLinear;
  VkSampler _defaultSamplerNearest;


  VkDescriptorSetLayout _gpuSceneDataDescriptorLayout;
  VkDescriptorSetLayout _singleImageDescriptorLayout;

  //> inst_init
  VkInstance _instance;                       // Vulkan library handle
  VkDebugUtilsMessengerEXT _debug_messenger;  // Vulkan debug output handle
  VkPhysicalDevice _chosenGPU;  // GPU chosen as the default device
  VkDevice _device;             // Vulkan device for commands
  VkSurfaceKHR _surface;        // Vulkan window surface
                                //< inst_init

  DescriptorAllocatorGrowable globalDescriptorAllocator;

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

  //< mesh pipeline
  VkPipelineLayout _meshPipelineLayout;
  VkPipeline _meshPipeline;

  std::vector<std::shared_ptr<MeshAsset>> testMeshes;
  MaterialInstance defaultData;
  GLTFMetallic_Roughness metalRoughMaterial;

  std::unordered_map<std::string, std::shared_ptr<LoadedGLTF>> loadedScenes;


  void init_mesh_pipeline();
  //<mesh pipeline

  // initializes everything in the engine
  void init();

  // shuts down the engine
  void cleanup();

  void resize_swapchain();
  // draw loop
  void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);
  void draw();
  void draw_background(VkCommandBuffer cmd);

  GPUMeshBuffers uploadMesh(std::span<uint32_t> indices,
                            std::span<Vertex> vertices);
  AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage,
	  VmaMemoryUsage memoryUsage);
  void destroy_buffer(const AllocatedBuffer& buffer);


  // run main loop
  void run();
  void update_scene();

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
  void draw_geometry(VkCommandBuffer cmd);
  void draw_imgui(VkCommandBuffer cmd, VkImageView targetImageView);


  void init_default_data();
};
