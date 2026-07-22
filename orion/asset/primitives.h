#ifndef ORION_PRIMITIVES_H_
#define ORION_PRIMITIVES_H_

#include "glm/fwd.hpp"

struct Vert {
  glm::vec3 position;
  glm::vec3 normal;
};

struct Primitive {
  std::vector<Vert> vertices;
  std::vector<uint32_t> indices;
};

namespace primitives {
constexpr float kUnitCubeHalfSize = 0.5F;
Primitive cube();
}  // namespace primitives

#endif  // ORION_PRIMITIVES_H_
