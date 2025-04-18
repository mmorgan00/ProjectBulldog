#pragma once

#include <fmt/core.h>
#include <vk_mem_alloc.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

#include <array>
#include <deque>
#include <functional>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

enum class MaterialPass :uint8_t {
  MainColor,
  Transparent,
  Other
};
struct MaterialPipeline {
  VkPipeline pipeline;
  VkPipelineLayout layout;
};

struct MaterialInstance {
  MaterialPipeline* pipeline;
  VkDescriptorSet materialSet;
  MaterialPass passType;
};

struct DrawContext;

// base class for a renderable dynamic object
class IRenderable {

    virtual void Draw(const glm::mat4& topMatrix, DrawContext& ctx) = 0;
};

// implementation of a drawable scene node.
// the scene node can hold children and will also keep a transform to propagate
// to them
struct Node : public IRenderable {

    // parent pointer must be a weak pointer to avoid circular dependencies
    std::weak_ptr<Node> parent;
    std::vector<std::shared_ptr<Node>> children;

    glm::mat4 localTransform;
    glm::mat4 worldTransform;

    void refreshTransform(const glm::mat4& parentMatrix)
    {
        worldTransform = parentMatrix * localTransform;
        for (auto c : children) {
            c->refreshTransform(worldTransform);
        }
    }

    virtual void Draw(const glm::mat4& topMatrix, DrawContext& ctx)
    {
        // draw children
        for (auto& c : children) {
            c->Draw(topMatrix, ctx);
        }
    }
};


struct GPUSceneData {
  glm::mat4 view;
  glm::mat4 proj;
  glm::mat4 viewproj;
  glm::vec4 ambientColor;
  glm::vec4 sunlightDirection;  // w for sun power
  glm::vec4 sunlightColor;
};

struct AllocatedBuffer {
  VkBuffer buffer;
  VmaAllocation allocation;
  VmaAllocationInfo info;
};

// Interleaved due to alignment constraints
struct Vertex {
  glm::vec3 position;
  float uv_x;
  glm::vec3 normal;
  float uv_y;
  glm::vec4 color;
};

// holds the resources needed for a mesh
struct GPUMeshBuffers {
  AllocatedBuffer indexBuffer;
  AllocatedBuffer vertexBuffer;
  VkDeviceAddress vertexBufferAddress;
};

// push constants for our mesh object draws
struct GPUDrawPushConstants {
  glm::mat4 worldMatrix;
  VkDeviceAddress vertexBuffer;
};

struct GLTFMaterial {
	MaterialInstance data;
};

struct GeoSurface {
	uint32_t startIndex;
	uint32_t count;
	std::shared_ptr<GLTFMaterial> material;
};

struct MeshAsset {
  std::string name;

  std::vector<GeoSurface> surfaces;
  GPUMeshBuffers meshBuffers;
};

struct AllocatedImage {
  VkImage image;
  VkImageView imageView;
  VmaAllocation allocation;
  VkExtent3D imageExtent;
  VkFormat imageFormat;
};

struct DeletionQueue {
  std::deque<std::function<void()>> deletors;

  void push_function(std::function<void()>&& function) {
    deletors.push_back(function);
  }

  void flush() {
    // reverse iterate the deletion queue to execute all the functions
    for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
      (*it)();  // call functors
    }

    deletors.clear();
  }
};

#define VK_CHECK(x)                                                  \
  do {                                                               \
    VkResult err = x;                                                \
    if (err) {                                                       \
      fmt::print("Detected Vulkan error: {}", string_VkResult(err)); \
      abort();                                                       \
    }                                                                \
  } while (0)
