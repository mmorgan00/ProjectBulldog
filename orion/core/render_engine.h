#ifndef ORION_CORE_RENDER_ENGINE_H_
#define ORION_CORE_RENDER_ENGINE_H_

ORION_CORE_RENDER_ENGINE_H_
#include <core/engine_types.h>

class RenderEngine{
  public:
  /**
   * @brief Initialize rendering resources, including render pipelines
   */
  virtual bool init(app_state& state) = 0;
  virtual ~RenderEngine() = default;
  virtual void draw() = 0;
  virtual void cleanup() = 0;
};

#endif // !CORE_RENDER_ENGINE_H_
