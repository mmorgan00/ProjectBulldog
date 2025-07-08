// Copyright 2025 Max Morgan

#ifndef ORION_CORE_RENDERER_H_
#define ORION_CORE_RENDERER_H_

#include <render_engine.h>

#include <memory>
#include <string>

class RenderComponent;

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

class RenderComponent {
 public:
  explicit RenderComponent(std::shared_ptr<RenderEngine> renderEngine);
  void setVisible();

 private:
  std::shared_ptr<RenderEngine> engine;
};

#endif  // ORION_CORE_RENDERER_H_
