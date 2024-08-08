#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <string_view>

class Instance {
	VkInstance _instance;

	bool checkValidationLayerSupport() const;
	std::vector<const char*> getRequiredExtensions();

public:
	Instance(std::string_view engineName);
	~Instance();
	const VkInstance getVkInstance() const;
	std::vector<VkPhysicalDevice> getAvailablePhysicalDevices() const;
};