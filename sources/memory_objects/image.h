#pragma once

#include "logical_device/logical_device.h"

#include <vulkan/vulkan.h>

struct SwapchainImage {
	VkImage _resourcesImage;
	VkDeviceMemory _resourcesMemory;
	VkImageView _resourcesView;
};

void transitionImageLayout(LogicalDevice* logicalDevice, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
void copyBufferToImage(LogicalDevice* logicalDevice, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
void generateMipmaps(LogicalDevice* logicalDevice, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);