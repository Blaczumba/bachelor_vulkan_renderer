#pragma once

#include "logical_device/logical_device.h"
#include "memory_objects/image.h"

#include <memory>

class Texture {
protected:
	Image _image;
	uint32_t _mipLevels;
	uint32_t _layerCount;

	std::shared_ptr<LogicalDevice> _logicalDevice;
public:
	Texture(std::shared_ptr<LogicalDevice> logicalDevice);
	virtual ~Texture() = default;

	const Image& getImage() const;

	void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout);
};