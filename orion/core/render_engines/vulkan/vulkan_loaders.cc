// Copyright 2025 Max Morgan

#include "orion/core/render_engines/vulkan/vulkan_loaders.h"

#include <vector>
#include <filesystem>
#include <utility>
#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>

#include "orion/util/logger.h"

std::optional<std::vector<std::shared_ptr<MeshAsset>>> vkutil::loadMeshGLB(
    VulkanEngine* engine, std::filesystem::path filePath) {
  // Create a GltfDataBuffer and load file data
  auto data = fastgltf::GltfDataBuffer::FromPath(filePath);
  // Set up parser and options
  fastgltf::Parser parser;
  fastgltf::Asset gltf;
  constexpr auto gltfOptions = fastgltf::Options::LoadExternalBuffers;
  // Load the GLB file
  auto load =
      parser.loadGltfBinary(data.get(), filePath.parent_path(), gltfOptions);
  if (load) {
    gltf = std::move(load.get());
    OE_LOG(RENDERER, INFO, "Opened {} successfully", filePath.c_str());
  } else {
    fmt::print("Failed to load GLB: {}\n",
               fastgltf::to_underlying(load.error()));
    return {};
  }

  // Parse gltf into formats we need
  std::vector<std::shared_ptr<MeshAsset>> meshes;
  // use the same vectors for all meshes so that the memory doesnt reallocate as
  // often
  std::vector<uint32_t> indices;
  std::vector<Vertex> vertices;
  for (fastgltf::Mesh& mesh : gltf.meshes) {
    MeshAsset newmesh;

    newmesh.name = mesh.name;

    // clear the mesh arrays each mesh, we dont want to merge them by error
    indices.clear();
    vertices.clear();

    for (fastgltf::Primitive& p : mesh.primitives) {
      GeoSurface newSurface;
      newSurface.startIndex = static_cast<uint32_t>(indices.size());
      newSurface.count = static_cast<uint32_t>(
          gltf.accessors[p.indicesAccessor.value()].count);

      size_t initial_vtx = vertices.size();

      // load indexes
      {
        fastgltf::Accessor& indexaccessor =
            gltf.accessors[p.indicesAccessor.value()];
        indices.reserve(indices.size() + indexaccessor.count);

        fastgltf::iterateAccessor<std::uint32_t>(
            gltf, indexaccessor,
            [&](std::uint32_t idx) { indices.push_back(idx + initial_vtx); });

        OE_LOG(RENDERER, INFO, "NUM INDICIES {}", indices.size());
      }

      // load vertex positions
      {
        // findAttribute returns a iterator into the underlying vector of
        // primitive attributes. Note that the glTF spec requires every
        // primitive to have a POSITION, so it's perfectly valid to assert that
        // positionIt is never nullptr.
        auto positionIt = p.findAttribute("POSITION");
        fastgltf::Accessor& positionaccessor =
            gltf.accessors[positionIt->accessorIndex];

        vertices.resize(vertices.size() + positionaccessor.count);

        fastgltf::iterateAccessorWithIndex<glm::vec3>(
            gltf, positionaccessor, [&](glm::vec3 v, size_t index) {
              Vertex newvtx;
              newvtx.position = v;
              newvtx.normal = {1, 0, 0};
              newvtx.color = glm::vec4{1.f};
              newvtx.uv_x = 0;
              newvtx.uv_y = 0;
              vertices[initial_vtx + index] = newvtx;
            });

        OE_LOG(RENDERER, INFO, "NUM VERTS {}", vertices.size());
      }
      {
        // load vertex normals
        auto normals = p.findAttribute("NORMAL");
        fastgltf::Accessor& normalAccessor =
            gltf.accessors[normals->accessorIndex];

        if (normals != p.attributes.end()) {
          fastgltf::iterateAccessorWithIndex<glm::vec3>(
              gltf, normalAccessor, [&](glm::vec3 v, size_t index) {
                vertices[initial_vtx + index].normal = v;
              });
        }
      }

      /**
      // load UVs
      auto uv = p.findAttribute("TEXCOORD_0");
      if (uv != p.attributes.end()) {
        fastgltf::iterateAccessorWithIndex<glm::vec2>(
            gltf, gltf.accessors[(*uv).second], [&](glm::vec2 v, size_t index)
    { vertices[initial_vtx + index].uv_x = v.x; vertices[initial_vtx +
    index].uv_y = v.y;
            });
      }

      // load vertex colors
      auto colors = p.findAttribute("COLOR_0");
      if (colors != p.attributes.end()) {
        fastgltf::iterateAccessorWithIndex<glm::vec4>(
            gltf, gltf.accessors[(*colors).second],
            [&](glm::vec4 v, size_t index) {
              vertices[initial_vtx + index].color = v;
            });
      }
      newmesh.surfaces.push_back(newSurface);
    }
    **/
    }
    // display the vertex normals
    constexpr bool OverrideColors = true;
    if (OverrideColors) {
      for (Vertex& vtx : vertices) {
        vtx.color = glm::vec4(vtx.normal, 1.f);
      }
    }
    // OE_LOG(RENDERER, INFO, "Uploading meshes");
    // newmesh.meshBuffers = engine->uploadMesh(indices, vertices);

    // meshes.emplace_back(std::make_shared<MeshAsset>(std::move(newmesh)));
  }

  return meshes;
}
