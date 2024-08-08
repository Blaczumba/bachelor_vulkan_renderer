#pragma once

#include <vulkan/vulkan.h>

#include <memory>

class LogicalDevice;

class SingleTimeCommandBuffer {
	VkCommandBuffer _commandBuffer;
	VkFence _fence;

	const LogicalDevice& _logicalDevice;

public:
	SingleTimeCommandBuffer(const LogicalDevice& logicalDevice);
	~SingleTimeCommandBuffer();

	VkCommandBuffer getCommandBuffer() const;
};

void copyBuffer(const LogicalDevice& logicalDevice, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);