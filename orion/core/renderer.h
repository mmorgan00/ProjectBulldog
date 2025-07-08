// Copyright 2025 Max Morgan

#ifndef ORION_CORE_RENDERER_H_
#define ORION_CORE_RENDERER_H_

#include <memory>
#include <string>
#include "core/engine_types.h"

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
   * @brief load a single object to be rendered
   */
  std::shared_ptr<RenderComponent> loadObject();

  /**
   * @brief loads a scene by name
   */
  void loadScene(std::string sceneName);

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
  virtual void loadScene() = 0;
  virtual void draw() = 0;
  virtual void cleanup() = 0;
};

class RenderComponent {
 public:
  explicit RenderComponent(std::shared_ptr<RenderEngine> renderEngine);
  void setVisible();

 private:
  std::shared_ptr<RenderEngine> engine;
};

#endif  // ORION_CORE_RENDERER_H_
