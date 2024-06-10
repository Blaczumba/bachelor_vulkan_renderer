#pragma once

#include "instance/instance.h"

#include <vulkan/vulkan.h>
#include <memory>

class DebugMessenger {
	VkDebugUtilsMessengerEXT _debugMessenger;
	std::shared_ptr<Instance> _instance;

	VkResult CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator);
	bool enableValidationLayers = true;
public:
	DebugMessenger(std::shared_ptr<Instance> instance);
	~DebugMessenger();
};