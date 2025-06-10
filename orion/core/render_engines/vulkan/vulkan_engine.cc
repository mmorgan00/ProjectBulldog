#include "vulkan_engine.h"
#include <core/engine_types.h>

#include <fmt/printf.h>


VulkanEngine* loadedEngine = nullptr;

bool VulkanEngine::init(app_state& state){
  return true;
}

void VulkanEngine::cleanup(){}

void VulkanEngine::draw(){
};


