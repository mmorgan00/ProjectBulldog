#ifndef ORION_ASSET_PRIMITIVES_CC_
#define ORION_ASSET_PRIMITIVES_CC_

#include "orion/asset/primitives.h"

#include "glm/ext/vector_float4.hpp"

Primitive primitives::cube() {
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
        {.position = coord - face.u * scale - face.v * scale,
         .uv_x = 0.0F,
         .normal = face.normal,
         .uv_y = 0.0F,
         .color = glm::vec4(1.0F)});
    cube.vertices.push_back(
        {.position = coord + face.u * scale - face.v * scale,
         .uv_x = 0.0F,
         .normal = face.normal,
         .uv_y = 0.0F,
         .color = glm::vec4(1.0F)});
    cube.vertices.push_back(
        {.position = coord + face.u * scale + face.v * scale,
         .uv_x = 0.0F,
         .normal = face.normal,
         .uv_y = 0.0F,
         .color = glm::vec4(1.0F)});
    cube.vertices.push_back(
        {.position = coord - face.u * scale + face.v * scale,
         .uv_x = 0.0F,
         .normal = face.normal,
         .uv_y = 0.0F,
         .color = glm::vec4(1.0F)});
    // two triangles, CCW
    cube.indices.insert(cube.indices.end(),
                        {base, base + 1, base + 2, base + 2, base + 3, base});
  }
  return cube;
}

#endif  // ORION_ASSET_PRIMITIVES_CC_
