#pragma once

#include "memory_objects/image.h"

#include <vulkan/vulkan.h>

#include <memory>

class LogicalDevice;

struct Texture {
	VkImage _image				= VK_NULL_HANDLE;
	VkDeviceMemory _memory		= VK_NULL_HANDLE;
	VkImageView _view			= VK_NULL_HANDLE;
	VkFormat _format			= VK_FORMAT_UNDEFINED;
	VkImageLayout _layout		= VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageAspectFlags _aspect	= VK_IMAGE_ASPECT_NONE;
	uint32_t _width				= 0u;
	uint32_t _height			= 0u;
	uint32_t _depth				= 0u;
	uint32_t _mipLevels			= 0u;
	uint32_t _layerCount		= 1u;

	virtual ~Texture() = default;

	void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout);

protected:
	void setParameters(VkFormat format, VkImageLayout layout, VkImageAspectFlags aspect, uint32_t mipLevels = 1u, uint32_t layerCount = 1u, uint32_t width = 1u, uint32_t height = 1u, uint32_t depth = 1u);
	void generateMipmaps(VkCommandBuffer commandBuffer);
};