#pragma once

#include "physical_device/physical_device.h"

#include <vulkan/vulkan.h>

#include <memory>

class LogicalDevice {
	VkDevice _device;
	std::shared_ptr<PhysicalDevice> _physicalDevice;
public:
	LogicalDevice(std::shared_ptr<PhysicalDevice> physicalDevice);
	~LogicalDevice();
	VkDevice getVkDevice() const;

	VkQueue graphicsQueue;
	VkQueue presentQueue;
};