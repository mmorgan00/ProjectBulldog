#include "vulkan_backend.hpp"

#include <iostream>
#include <vulkan/vulkan_core.h>

#include <SDL.h>
#include <SDL_vulkan.h>
#include "VkBootstrap.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"


bool VulkanRendererBackend::init(){
   // Init SDL
  // Initialize SDL
  // TODO: Tuck into a 'platform init'
  std::cout << "Initializing platform window" << std::endl;
  SDL_Init(SDL_INIT_VIDEO);
  SDL_SetRelativeMouseMode(SDL_TRUE);

  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
  _window = SDL_CreateWindow("ProjectBulldog", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _windowExtent.width, _windowExtent.height, window_flags);

  std::cout << "Renderer initializing" << std::endl;
  
  // Initialize vulkan
	//> init_instance
	vkb::InstanceBuilder builder;

	// make the vulkan instance, with basic debug features
	auto inst_ret = builder.set_app_name("Project Bulldog")
		.request_validation_layers(VK_TRUE) // TODO: Make configurable
		.use_default_debug_messenger()
		.require_api_version(1, 3, 0)
		.build();

	vkb::Instance vkb_inst = inst_ret.value();
  if(!vkb_inst){
    std::cout << "Unable to initialize vulkan" << std::endl;
    return false;
  }
	// grab the instance
	_instance = vkb_inst.instance;
	_debug_messenger = vkb_inst.debug_messenger;

	//< init_instance
  std::cout << "Vulkan instance created" << std::endl;
	//
	//> init_device
	SDL_Vulkan_CreateSurface(_window, _instance, &_surface);

  std::cout << "SDL/Vulkan surface created" << std::endl;
	// vulkan 1.3 features
	VkPhysicalDeviceVulkan13Features features{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	features.dynamicRendering = true;
	features.synchronization2 = true;

	// vulkan 1.2 features
	VkPhysicalDeviceVulkan12Features features12{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
	features12.bufferDeviceAddress = true;
	features12.descriptorIndexing = true;

	// use vkbootstrap to select a gpu.
	// We want a gpu that can write to the SDL surface and supports vulkan 1.3
	// with the correct features
	vkb::PhysicalDeviceSelector selector{ vkb_inst };
	vkb::PhysicalDevice physicalDevice = selector.set_minimum_version(1, 3)
		.set_required_features_13(features)
		.set_required_features_12(features12)
		.set_surface(_surface)
		.select()
		.value();
 std::cout << "Physical device selected" << std::endl;

	// create the final vulkan device
	vkb::DeviceBuilder deviceBuilder{ physicalDevice };

	vkb::Device vkbDevice = deviceBuilder.build().value();

  std::cout << "Physical device selected" << std::endl;

	// Get the VkDevice handle used in the rest of a vulkan application
	_device = vkbDevice.device;
	_chosenGPU = physicalDevice.physical_device;
	//< init_device

	//> init_queue
	// use vkbootstrap to get a Graphics queue
	_graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	_graphicsQueueFamily =
		vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
	//< init_queue
  std::cout << "Queues selected" << std::endl;

	//> init allocator
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = _chosenGPU;
	allocatorInfo.device = _device;
	allocatorInfo.instance = _instance;
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	vmaCreateAllocator(&allocatorInfo, &_allocator);
	//< init allocator
	// _mainDeletionQueue.push_function([&]() { vmaDestroyAllocator(_allocator); });


  return true;
}

// TODO: Vulkan deletion queue
VulkanRendererBackend::~VulkanRendererBackend(){

}
