// Copyright 2025 Max Morgan

#ifndef ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_ENGINE_H_
#define ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_ENGINE_H_
#include <core/renderer.h>
#include <vulkan/vulkan_core.h>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "core/engine_types.h"
#include "core/render_engines/vulkan/vulkan_descriptors.h"
#include "core/render_engines/vulkan/vulkan_types.h"
#include "entity/camera.h"
#include "util/logger.h"

DECLARE_LOG_CATEGORY(VULKAN_ENGINE);

struct GPUSceneData {
  glm::mat4 view;
  glm::mat4 proj;
  glm::mat4 viewproj;
  glm::vec4 ambientColor;
  glm::vec4 sunlightDirection;  // w for sun power
  glm::vec4 sunlightColor;
};

struct MeshNode : public Node {
  std::shared_ptr<MeshAsset> mesh;

  void Draw(const glm::mat4& topMatrix, DrawContext& ctx) override;
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

class VulkanEngine;

struct GLTFMetallic_Roughness {
  MaterialPipeline opaquePipeline;
  MaterialPipeline transparentPipeline;

  VkDescriptorSetLayout materialLayout;

  struct MaterialConstants {
    glm::vec4 colorFactors;
    glm::vec4 metal_rough_factors;
    // padding, we need it anyway for uniform buffers
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

  MaterialInstance write_material(
      VkDevice device, MaterialPass pass, const MaterialResources& resources,
      DynamicDescriptorAllocator& descriptorAllocator);
};

class LoadedGLTF;  // Engine needs to know about it for storing. Declared later

class VulkanEngine : public RenderEngine {
  bool _isInitialized{false};
  int _frameNumber{0};
  bool stop_rendering{false};
  VkExtent2D _windowExtent{2580, 1600};
  // To select the correct sync and command objects, we need to keep track of
  // the current frame and (swapchain) image index
  uint32_t currentFrame{0};
  uint32_t currentSemaphore{0};

  struct SDL_Window* _window{nullptr};

  //< Object managers
  VmaAllocator _allocator;
  //> Object managers

  //< Vk resource handles
  VkInstance _instance;                       // Vulkan library handle
  VkDebugUtilsMessengerEXT _debug_messenger;  // Vulkan debug output handle
  VkPhysicalDevice _chosenGPU;  // GPU chosen as the default device
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
  void resize_swapchain();
  void init_commands();
  void init_sync_structures();
  void init_descriptors();
  void init_pipelines();
  void init_background_pipeline();
  void init_default_pipeline();
  void init_default_data();

  void set_camera(Camera* camera) override;

  // Default data
  std::vector<std::shared_ptr<MeshAsset>> meshes;

  GPUSceneData sceneData;

  Camera* mainCamera;

  // Immediates
  VkFence _immFence;
  VkCommandBuffer _immCommandBuffer;
  VkCommandPool _immCommandPool;

  void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);

  // Swapchain
  VkSwapchainKHR _swapchain;
  VkFormat _swapchainImageFormat;

  void create_swapchain(uint32_t width, uint32_t height);
  void destroy_swapchain();

  // Buffer management
  std::vector<VkImage> _swapchainImages;
  std::vector<VkImageView> _swapchainImageViews;
  VkExtent2D _swapchainExtent;

  DrawContext mainDrawContext;
  std::unordered_map<std::string, std::shared_ptr<Node>> loadedNodes;

  void update_scene();

 public:
  bool resize_requested{false};
  // Descriptor sets
  DynamicDescriptorAllocator globalDescriptorAllocator;

  VkDescriptorSet _drawImageDescriptors;
  VkDescriptorSetLayout _drawImageDescriptorLayout;

  // Pipelines
  VkPipeline _gradientPipeline;
  VkPipelineLayout _gradientPipelineLayout;

  VkPipelineLayout _defaultPipelineLayout;
  VkPipeline _defaultPipeline;

  bool init(app_state& state) override;
  void loadScene(std::string_view fileName) override;
  std::shared_ptr<RenderComponent> loadObject() override;


  DeletionQueue _mainDeletionQueue;

  // TODO: A good amount of these should not be exposed publicly based on the
  // frontend/backend setup we have already. THis is just to get GLTF file
  // loading working
  AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage,
                                VmaMemoryUsage memoryUsage);

  void destroy_buffer(const AllocatedBuffer& buffer);

  // Image management
  AllocatedImage create_image(VkExtent3D size, VkFormat format,
                              VkImageUsageFlags usage, bool mipmapped = false);
  AllocatedImage create_image(void* data, VkExtent3D size, VkFormat format,
                              VkImageUsageFlags usage, bool mipmapped = false);
  void destroy_image(const AllocatedImage& img);

  // End TODO

  std::unordered_map<std::string, std::shared_ptr<LoadedGLTF>> loadedScenes;
  VkSampler _defaultSamplerLinear;
  VkSampler _defaultSamplerNearest;

  VkDevice _device;  // Vulkan device for commands

  //< Render loop resources
  AllocatedImage _drawImage;
  AllocatedImage _depthImage;
  VkExtent2D _drawExtent;
  VkExtent2D _drawImageExtent;
  //> Render loop resources

  MaterialInstance defaultData;
  GLTFMetallic_Roughness metalRoughMaterial;

  AllocatedImage _whiteImage;
  AllocatedImage _blackImage;
  AllocatedImage _greyImage;
  AllocatedImage _errorCheckerboardImage;

  // Layouts
  VkDescriptorSetLayout
      _gpuSceneDataDescriptorLayout;  // Generic layout for any drawable object
  VkDescriptorSetLayout
      _singleImageDescriptorLayout;  // Single Image input shader pipeline

  void resize_window() override;
  // draw loop
  void draw() override;
  void draw_background(VkCommandBuffer cmd);
  void draw_geometry(VkCommandBuffer cmd);
  // shuts down the engine
  void cleanup() override;

  GPUMeshBuffers uploadMesh(std::span<uint32_t> indices,
                            std::span<Vertex> vertices);
};

struct LoadedGLTF : IRenderable {
  // storage for all the data on a given glTF file
  std::unordered_map<std::string, std::shared_ptr<MeshAsset>> meshes;
  std::unordered_map<std::string, std::shared_ptr<Node>> nodes;
  std::unordered_map<std::string, AllocatedImage> images;
  std::unordered_map<std::string, std::shared_ptr<GLTFMaterial>> materials;

  // nodes that dont have a parent, for iterating through the file in tree order
  std::vector<std::shared_ptr<Node>> topNodes;

  std::vector<VkSampler> samplers;

  DynamicDescriptorAllocator descriptorPool;

  AllocatedBuffer materialDataBuffer;

  VulkanEngine* creator;

  ~LoadedGLTF() { clearAll(); }

  virtual void Draw(const glm::mat4& topMatrix, DrawContext& ctx);

 private:
  void clearAll();
};

#endif  // ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_ENGINE_H_
