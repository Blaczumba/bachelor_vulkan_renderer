#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <string_view>

class Instance {
	VkInstance _instance;

	bool checkValidationLayerSupport() const;

public:
	Instance(std::string_view engineName, const std::vector<const char*>& requiredExtensions);
	~Instance();
	const VkInstance getVkInstance() const;
	std::vector<VkPhysicalDevice> getAvailablePhysicalDevices() const;
};