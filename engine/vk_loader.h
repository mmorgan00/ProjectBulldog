#include <engine_types.h>

#include <filesystem>
#include <unordered_map>

// forward declaration
class VulkanEngine;

std::optional<std::vector<std::shared_ptr<MeshAsset>>> loadGltfMeshes(
    VulkanEngine* engine, std::filesystem::path filePath);
