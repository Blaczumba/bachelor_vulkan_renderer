#pragma once

#include "logical_device/logical_device.h"

#include <vulkan/vulkan.h>

struct SwapchainImage {
	VkImage _resourcesImage;
	VkDeviceMemory _resourcesMemory;
	VkImageView _resourcesView;
};

void transitionImageLayout(LogicalDevice* logicalDevice, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectFlags, uint32_t mipLevels);