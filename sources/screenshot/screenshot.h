#pragma once

#include "physical_device/device/physical_device.h"
#include "swapchain/swapchain.h"
#include "memory_objects/image.h"

#include <vulkan/vulkan.h>

#include <memory>

class Screenshot {
	std::shared_ptr<LogicalDevice> _logicalDevice;
public:
	Screenshot(std::shared_ptr<LogicalDevice> logicalDevice);
	void saveImage(const std::string& filePath, const Image& image);
};
