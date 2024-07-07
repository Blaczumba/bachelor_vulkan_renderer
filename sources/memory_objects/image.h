#pragma once

#include "logical_device/logical_device.h"

#include <vulkan/vulkan.h>

class Image {
protected:
	VkImage _image;
	VkFormat _format;
	VkDeviceMemory _memory;
	VkImageView _view;
	VkExtent2D _extent;
public:
	Image(VkFormat format);

	VkImage getVkImage() const;
	VkFormat getVkFormat() const;
	VkDeviceMemory getVkDeviceMemory() const;
	VkImageView getVkImageView() const;
	VkExtent2D getVkExtent() const;

	virtual ~Image() = default;
};

void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
void copyImageToBuffer(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout layout, VkBuffer buffer, uint32_t width, uint32_t height);
void generateMipmaps(VkCommandBuffer commandBuffer, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);