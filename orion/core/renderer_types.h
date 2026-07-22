#ifndef RENDERER_TYPES_H_
#define RENDERER_TYPES_H_

#include <cstdint>

#include "orion/asset/primitives.h"

// Renderer frontend types are very similar to backend types however they hold
// raw data, specialized types are per-backend specific
// Materials are meaningless for now and just a placeholder type. Everything
// will be default material

namespace engine {
struct MeshBuffers {
  std::vector<uint32_t> indexBuffer;
  std::vector<Vert> vertexBuffer;
};

struct Material {
  uint64_t index = 0;
};

struct Surface {
  uint32_t startIndex = 0;
  uint32_t count = 0;
  std::shared_ptr<Material> material;
};

struct MeshAsset {
  std::string name;
  std::vector<engine::Surface> surfaces;
  MeshBuffers meshBuffers;
};
}  // namespace engine
#endif  // RENDERER_TYPES_H_
