#pragma once
class RendererBackend{
  public:
  /**
   * @brief Initialize rendering resources, including render pipelines
   */
  virtual bool init() = 0;
  virtual ~RendererBackend() = default;
  virtual void shutdown() = 0;
};
