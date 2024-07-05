#pragma once

#include "physical_device/device/physical_device.h"
#include "swapchain/swapchain.h"

#include <vulkan/vulkan.h>

#include <memory>

class Screenshot {
	bool _blittingEnabled;

	std::shared_ptr<LogicalDevice> _logicalDevice;
	std::shared_ptr<Swapchain> _swapchain;
public:
	Screenshot(std::shared_ptr<PhysicalDevice> physicalDevice, std::shared_ptr<LogicalDevice> logicalDevice, std::shared_ptr<Swapchain> swapchain);
	void saveScreenshot(const std::string& filepath, uint32_t imageIndex);
	void saveImage(const std::string& filepath, VkImage image, VkExtent2D extent);
	// void saveScreenshot(const std::string& filepath, )
};
