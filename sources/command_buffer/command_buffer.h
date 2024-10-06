#pragma once

#include <vulkan/vulkan.h>

#include <memory>

class LogicalDevice;
class CommandBuffer;

class CommandPool {
	VkCommandPool _commandPool;

	const LogicalDevice& _logicalDevice;

public:
	CommandPool(const LogicalDevice& logicalDevice);
	~CommandPool();

	std::unique_ptr<CommandBuffer> createPrimaryCommandBuffer() const;
	std::unique_ptr<CommandBuffer> createSecondaryCommandBuffer() const;
	// std::vector<std::unique_ptr<CommandBuffer>> createCommandBuffers(uint32_t count) const;

	const VkCommandPool getVkCommandPool() const;

private:
	const LogicalDevice& getLogicalDevice() const;
	friend class CommandBuffer;
};

class CommandBuffer {
	VkCommandBuffer _commandBuffer;

	const CommandPool& _commandPool;
public:
	CommandBuffer(const CommandPool& commandPool, bool primary = true);
	void resetCommandBuffer() const;
	const VkCommandBuffer getVkCommandBuffer() const;
};

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