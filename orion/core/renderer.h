// Copyright 2025 Max Morgan

#ifndef ORION_CORE_RENDERER_H_
#define ORION_CORE_RENDERER_H_

#include <memory>
#include <string_view>

#include "orion/core/engine_types.h"
#include "orion/core/renderer_types.h"
#include "orion/core/resource_types/static_mesh.h"
#include "orion/entity/camera.h"
// Forward declaring here, each RendererBackend will have it's own
// implementation (GPU allocation types)
class RenderEngine;

class Renderer {
 public:
  /**
   * @brief initializes any render specific resources
   */
  void init(AppState& state);
  /**
   * @brief Cleans up any rendering specific resources
   */
  void cleanup();
  /**
   * @brief Draws the next frame
   */
  void draw(engine::DrawContext);

  /**
   * @brief Load a single static mesh
   **/
  RenderObject* loadStaticMesh(StaticMesh* mesh);

  /**
   * @brief resize the window
   */
  void resize_window();

  /**
   * @brief Sets the camera to use fr the render view matrix
   */
  void set_camera(Camera* camera);

 private:
  std::shared_ptr<RenderEngine> engine;
};

class RenderEngine {
 public:
  /**
   * @brief Initialize rendering resources, including render pipelines
   */
  virtual bool init(AppState& state) = 0;
  virtual ~RenderEngine() = default;
  virtual void loadObject(engine::MeshAsset object) = 0;
  virtual RenderObject* uploadMesh(engine::MeshAsset mesh) = 0;
  virtual void loadScene(std::string_view fileName) = 0;
  // virtual std::shared_ptr<RenderComponent> loadObject() = 0;
  virtual void set_camera(Camera* camera) = 0;
  virtual void draw(engine::DrawContext) = 0;
  virtual void cleanup() = 0;
  virtual void resize_window() = 0;
  bool resize_requested{false};
};

// class RenderComponent {
//  public:
//   explicit RenderComponent(RenderEngine* renderEngine);

//  private:
//   RenderEngine* engine;
// };

#endif  // ORION_CORE_RENDERER_H_
