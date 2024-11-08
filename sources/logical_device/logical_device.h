#pragma once

#include "physical_device/physical_device.h"
#include "memory_objects/buffers.h"

#include <memory>

enum class QueueType : uint8_t {
	GRAPHICS = 0,
	PRESENT,
	COMPUTE,
	TRANSFER
};

class LogicalDevice {
	VkDevice _device;

	const PhysicalDevice& _physicalDevice;

	VkQueue _graphicsQueue;
	VkCommandPool _graphicsCommandPool;
	VkQueue _presentQueue;
	VkCommandPool _presentCommandPool;
	VkQueue _computeQueue;
	VkCommandPool _computeCommandPool;
	VkQueue _transferQueue;
	VkCommandPool _transferCommandPool;

public:
	LogicalDevice(const PhysicalDevice& physicalDevice);
	~LogicalDevice();

	const VkCommandPool getCommandPool(QueueType queueType) const;
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
	void createImage(Image* image) const;
	void createImageView(Image* image) const;
	void createSampler(Sampler* sampler) const;

	const VkDevice getVkDevice() const;
	const PhysicalDevice& getPhysicalDevice() const;

	const VkQueue getQueue(QueueType queueType) const;
	const VkQueue getGraphicsQueue() const;
	const VkQueue getPresentQueue() const;
	const VkQueue getComputeQueue() const;
	const VkQueue getTransferQueue() const;

private:
	VkCommandPool createSingleSubmitCommandPool(uint32_t queueFamilyIndex);

	friend class SingleTimeCommandBuffer;
	const VkCommandPool getSingleSubmitCommandPool(QueueType queueType) const;
};
