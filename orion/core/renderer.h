// Copyright 2025 Max Morgan

#ifndef ORION_CORE_RENDERER_H_
#define ORION_CORE_RENDERER_H_

#include <memory>
#include <string>
#include <string_view>

#include "core/engine_types.h"
#include "entity/camera.h"

class RenderComponent;
class RenderEngine;

class Renderer {
 public:
  /**
   * @brief initializes any render specific resources
   */
  void init(app_state& state);
  /**
   * @brief Cleans up any rendering specific resources
   */
  void cleanup();
  /**
   * @brief Draws the next frame
   */
  void draw();

  /**
   * @brief load a single Scene to be rendered
   */
  std::shared_ptr<RenderComponent> loadScene(std::string_view fileName);

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
  virtual bool init(app_state& state) = 0;
  virtual ~RenderEngine() = default;
  virtual void loadScene(std::string_view fileName) = 0;
  virtual std::shared_ptr<RenderComponent> loadObject() = 0;
  virtual void set_camera(Camera* camera) = 0;
  virtual void draw() = 0;
  virtual void cleanup() = 0;
  virtual void resize_window() = 0;
  bool resize_requested{false};
};

class RenderComponent {
 public:
  explicit RenderComponent(RenderEngine* renderEngine);

 private:
  RenderEngine* engine;
};

#endif  // ORION_CORE_RENDERER_H_
