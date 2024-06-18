#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string_view>

class Instance {
	VkInstance _instance;

	bool checkValidationLayerSupport() const;
	std::vector<const char*> getRequiredExtensions();
public:
	Instance(std::string_view engineName);
	~Instance();
	VkInstance getVkInstance();
	std::vector<VkPhysicalDevice> getAvailablePhysicalDevices() const;
};