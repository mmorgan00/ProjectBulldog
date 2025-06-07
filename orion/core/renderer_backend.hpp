#pragma once

#include "core/engine_types.hpp"

class RendererBackend{
  public:
  /**
   * @brief Initialize rendering resources, including render pipelines
   */
  virtual bool init(app_state& state) = 0;
  virtual ~RendererBackend() = default;
  virtual void shutdown() = 0;
  virtual void run() = 0;
};
