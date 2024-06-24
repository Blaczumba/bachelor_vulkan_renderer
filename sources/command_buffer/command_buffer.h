#pragma once

#include "logical_device/logical_device.h"

#include <vulkan/vulkan.h>

class SingleTimeCommandBuffer {
	VkCommandBuffer _commandBuffer;
	LogicalDevice* _logicalDevice;
public:
	SingleTimeCommandBuffer(LogicalDevice* logicalDevice);
	~SingleTimeCommandBuffer();

	VkCommandBuffer getCommandBuffer() const;
};

void copyBuffer(LogicalDevice* logicalDevice, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);