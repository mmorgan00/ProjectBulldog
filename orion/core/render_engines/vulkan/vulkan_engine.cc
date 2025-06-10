// Copyright 2025 Max Morgan

#include "orion/core/render_engines/vulkan/vulkan_engine.h"

#include <SDL.h>
#include <SDL_vulkan.h>
#include <core/engine_types.h>
#include <fmt/printf.h>

VulkanEngine* loadedEngine = nullptr;

bool VulkanEngine::init(app_state& state) {
  _isInitialized = true;
  return true;
}

void VulkanEngine::cleanup() {
  if (_isInitialized) {
    SDL_DestroyWindow(_window);
  }

  // clear engine pointer
  loadedEngine = nullptr;
}

void VulkanEngine::draw() {}
