#pragma once

#include <vulkan/vulkan.h>

#include <vector>

struct Image {
	VkImage image				= VK_NULL_HANDLE;
	VkDeviceMemory memory		= VK_NULL_HANDLE;
	VkImageView view			= VK_NULL_HANDLE;
	VkFormat format				= VK_FORMAT_UNDEFINED;
	VkImageLayout layout		= VK_IMAGE_LAYOUT_UNDEFINED;
	VkExtent3D extent			= {};
	VkImageAspectFlags aspect	= VK_IMAGE_ASPECT_NONE;
};

void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t layerCount = 1);
void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);	// Normal texture 2D
void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, std::vector<VkBufferImageCopy>&& regions);	// Cubemap
void copyImageToBuffer(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout layout, VkBuffer buffer, uint32_t width, uint32_t height);
void copyImageToImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImage dstImage, VkExtent2D srcSize, VkExtent2D dstSize, VkImageAspectFlagBits aspect = VK_IMAGE_ASPECT_COLOR_BIT);
void copyImageToImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImage dstImage, VkExtent2D extent, VkImageAspectFlagBits aspect = VK_IMAGE_ASPECT_COLOR_BIT);

