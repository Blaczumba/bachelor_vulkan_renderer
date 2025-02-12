#pragma once

#include <vulkan/vulkan.h>

#include "logical_device/logical_device.h"

#include <memory>

class CommandBuffer;
class Framebuffer;

class CommandPool {
	VkCommandPool _commandPool;

	const LogicalDevice& _logicalDevice;

public:
	CommandPool(const LogicalDevice& logicalDevice);
	~CommandPool();

	std::unique_ptr<CommandBuffer> createPrimaryCommandBuffer() const;
	std::unique_ptr<CommandBuffer> createSecondaryCommandBuffer() const;

	const VkCommandPool getVkCommandPool() const;
	const LogicalDevice& getLogicalDevice() const;
};

class CommandBuffer {
	VkCommandBuffer _commandBuffer;
	VkCommandBufferLevel _level;

	const CommandPool& _commandPool;

public:
	CommandBuffer(const CommandPool& commandPool, VkCommandBufferLevel level);
	~CommandBuffer();
	VkResult begin(const Framebuffer& framebuffer, VkCommandBufferUsageFlags flags = 0, uint32_t subpassIndex = 0) const;
	VkResult submit(QueueType type, const VkSemaphore waitSemaphore, const VkSemaphore signalSemaphore, const VkFence waitFence) const;
	void resetCommandBuffer() const;
	const VkCommandBuffer getVkCommandBuffer() const;
};

class SingleTimeCommandBuffer {
	VkCommandBuffer _commandBuffer;
	VkFence _fence;
	const QueueType _queueType;

	const CommandPool& _commandPool;

public:
	SingleTimeCommandBuffer(const CommandPool& commandPool, QueueType queueType = QueueType::GRAPHICS);
	~SingleTimeCommandBuffer();

	VkCommandBuffer getCommandBuffer() const;
};
