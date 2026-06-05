#ifndef ORION_PRIMITIVES_H_
#define ORION_PRIMITIVES_H_

#include "glm/fwd.hpp"

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
};

struct Primitive {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};

namespace primitives {
constexpr float kUnitCubeHalfSize = 0.5F;
Primitive cube() {
  // Each face: outward normal, then two in-plane edge vectors (u, v)
  // spanning the face. Corner = normal*half + (±u ± v)*half.
  struct Face {
    glm::vec3 normal, u, v;
  };
  constexpr float scale = 0.5F;

  const Face faces[6] = {
      {{0, 0, 1}, {1, 0, 0}, {0, 1, 0}},    // +Z
      {{0, 0, -1}, {-1, 0, 0}, {0, 1, 0}},  // -Z
      {{1, 0, 0}, {0, 0, -1}, {0, 1, 0}},   // +X
      {{-1, 0, 0}, {0, 0, 1}, {0, 1, 0}},   // -X
      {{0, 1, 0}, {1, 0, 0}, {0, 0, -1}},   // +Y
      {{0, -1, 0}, {1, 0, 0}, {0, 0, 1}},   // -Y
  };

  Primitive cube;
  for (const auto& face : faces) {
    uint32_t base = static_cast<uint32_t>(cube.vertices.size());
    glm::vec3 coord = face.normal * scale;
    cube.vertices.push_back(
        {coord - face.u * scale - face.v * scale, face.normal});
    cube.vertices.push_back(
        {coord + face.u * scale - face.v * scale, face.normal});
    cube.vertices.push_back(
        {coord + face.u * scale + face.v * scale, face.normal});
    cube.vertices.push_back(
        {coord - face.u * scale + face.v * scale, face.normal});
    // two triangles, CCW
    cube.indices.insert(cube.indices.end(),
                        {base, base + 1, base + 2, base + 2, base + 3, base});
  }
  return cube;
}
}  // namespace primitives

#endif  // ORION_PRIMITIVES_H_
