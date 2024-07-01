#pragma once

#include "physical_device/device/physical_device.h"

#include <memory>

class LogicalDevice {
	VkDevice _device;
	VkCommandPool _commandPool;

	std::shared_ptr<PhysicalDevice> _physicalDevice;
public:
	LogicalDevice(std::shared_ptr<PhysicalDevice> physicalDevice);
	~LogicalDevice();

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& outImage, VkDeviceMemory& outImageMemory);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	std::vector<VkCommandBuffer> createCommandBuffers(uint32_t commandBuffersCount);
	VkCommandBuffer createCommandBuffer();

	VkDevice getVkDevice() const;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	friend class SingleTimeCommandBuffer;
};