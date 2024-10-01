#pragma once

#include "physical_device/physical_device.h"
#include "memory_objects/image.h"

#include <memory>

class LogicalDevice {
	VkDevice _device;
	VkCommandPool _commandPool;

	const PhysicalDevice& _physicalDevice;

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

	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue computeQueue;
	VkQueue transferQueue;

	friend class SingleTimeCommandBuffer;
};