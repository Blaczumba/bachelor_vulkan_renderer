#pragma once

#include "physical_device/physical_device.h"
#include "memory_objects/texture/texture.h"

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
	VkQueue _presentQueue;
	VkQueue _computeQueue;
	VkQueue _transferQueue;

public:
	LogicalDevice(const PhysicalDevice& physicalDevice);
	~LogicalDevice();

	const VkBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage) const;
	const VkDeviceMemory createBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags properties) const;
	const VkImage createImage(const ImageParameters& params) const;
	const VkDeviceMemory createImageMemory(const VkImage image, const ImageParameters& params) const;
	const VkImageView createImageView(const VkImage image, const ImageParameters& params) const;
	const VkSampler createSampler(const SamplerParameters& params) const;

	const VkDevice getVkDevice() const;
	const PhysicalDevice& getPhysicalDevice() const;

	const VkQueue getQueue(QueueType queueType) const;
	const VkQueue getGraphicsQueue() const;
	const VkQueue getPresentQueue() const;
	const VkQueue getComputeQueue() const;
	const VkQueue getTransferQueue() const;
};
