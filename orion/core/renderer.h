#ifndef ORION_CORE_RENDERER_H_
#define ORION_CORE_RENDERER_H_

#include <render_engine.h>


class Renderer{
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
private:

  RenderEngine* engine;
};

#endif
