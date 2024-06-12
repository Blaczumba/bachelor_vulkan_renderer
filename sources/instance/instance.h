#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

class Instance {
	VkInstance _instance;
	const bool enableValidationLayers = true;

	bool checkValidationLayerSupport() const;
	std::vector<const char*> getRequiredExtensions();
public:
	Instance();
	~Instance();
	VkInstance getVkInstance();
	std::vector<VkPhysicalDevice> getAvailablePhysicalDevices() const;
};