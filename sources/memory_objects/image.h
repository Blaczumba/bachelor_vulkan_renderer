#pragma once

#include "logical_device/logical_device.h"

#include <vulkan/vulkan.h>

struct SwapchainImage {
	VkImage _resourcesImage;
	VkDeviceMemory _resourcesMemory;
	VkImageView _resourcesView;
	VkExtent2D _extent;
};

void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
void generateMipmaps(VkCommandBuffer commandBuffer, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);