#ifndef CORE_RESOURCE_TYPES_STATIC_MESH_H_
#define CORE_RESOURCE_TYPES_STATIC_MESH_H_

#include "orion/asset/primitives.h"

typedef struct StaticMesh {
 public:
  std::string name;
  std::vector<Vert> vertices;
  std::vector<uint32_t> indices;
} StaticMesh;

#endif  // CORE_RESOURCE_TYPES_STATIC_MESH_H_
