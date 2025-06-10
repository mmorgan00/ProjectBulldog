// Copyright 2025 Max Morgan

#ifndef ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_ENGINE_H_
#define ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_ENGINE_H_
#include <core/render_engine.h>

class VulkanEngine : public RenderEngine {
  bool _isInitialized{false};
  int _frameNumber{0};
  bool stop_rendering{false};
  VkExtent2D _windowExtent{1700, 900};

  struct SDL_Window* _window{nullptr};

 public:
  bool init(app_state& state) override;
  // draw loop
  void draw() override;
  // shuts down the engine
  void cleanup() override;
};
#endif  // ORION_CORE_RENDER_ENGINES_VULKAN_VULKAN_ENGINE_H_
