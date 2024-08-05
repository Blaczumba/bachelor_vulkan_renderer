#pragma once

#include "physical_device/physical_device.h"

#include <memory>

class LogicalDevice {
	VkDevice _device;
	VkCommandPool _commandPool;

	const PhysicalDevice& _physicalDevice;
public:
	LogicalDevice(const PhysicalDevice& physicalDevice);
	~LogicalDevice();

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& outImage, VkDeviceMemory& outImageMemory, uint32_t layerCount = 1);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t layerCount = 1);
	std::vector<VkCommandBuffer> createCommandBuffers(uint32_t commandBuffersCount) const;
	VkCommandBuffer createCommandBuffer() const;

	VkDevice getVkDevice() const;
	const PhysicalDevice& getPhysicalDevice() const;

	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue computeQueue;
	VkQueue transferQueue;

	friend class SingleTimeCommandBuffer;
};