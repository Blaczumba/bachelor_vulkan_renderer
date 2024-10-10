#pragma once

#include "physical_device/physical_device.h"
#include "memory_objects/buffers.h"

#include <memory>

class LogicalDevice {
	VkDevice _device;
	VkCommandPool _commandPool;

	const PhysicalDevice& _physicalDevice;

	VkQueue _graphicsQueue;
	VkQueue _presentQueue;
	VkQueue _computeQueue;
	VkQueue _transferQueue;

public:
	LogicalDevice(const PhysicalDevice& physicalDevice);
	~LogicalDevice();

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
	void createImage(Image* image) const;
	void createImageView(Image* image) const;
	void createSampler(Sampler* sampler) const;

	VkCommandBuffer createCommandBuffer() const;

	const VkDevice getVkDevice() const;
	const PhysicalDevice& getPhysicalDevice() const;

	const VkQueue getGraphicsQueue() const;
	const VkQueue getPresentQueue() const;
	const VkQueue getComputeQueue() const;
	const VkQueue getTransferQueue() const;

	friend class SingleTimeCommandBuffer;
};

enum class QueueType : uint8_t {
	GRAPHICS = 0,
	PRESENT,
	COMPUTE,
	TRANSFER
};
