#pragma once

#include "memory_objects/image.h"

#include <vulkan/vulkan.h>

#include <memory>

class LogicalDevice;

class Texture {
protected:
	Image _image;
	uint32_t _mipLevels;
	uint32_t _layerCount;

	const LogicalDevice& _logicalDevice;
public:
	Texture(const LogicalDevice& logicalDevice);
	virtual ~Texture() = default;

	const Image& getImage() const;

	void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout);
};