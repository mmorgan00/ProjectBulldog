#ifndef ORION_PRIMITIVES_H_
#define ORION_PRIMITIVES_H_

#include "glm/fwd.hpp"

struct Vertex {
  glm::vec3 position;
  float uv_x = 0.F;
  glm::vec3 normal;
  float uv_y = 0.F;
  glm::vec4 color;
};

struct Primitive {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};

namespace primitives {
constexpr float kUnitCubeHalfSize = 0.5F;
Primitive cube();
}  // namespace primitives

#endif  // ORION_PRIMITIVES_H_
