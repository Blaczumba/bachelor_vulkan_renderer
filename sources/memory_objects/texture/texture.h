#pragma once

#include "logical_device/logical_device.h"
#include "memory_objects/image.h"

#include <memory>

class Texture {
protected:
	Image _image;

	std::shared_ptr<LogicalDevice> _logicalDevice;
public:
	Texture(std::shared_ptr<LogicalDevice> logicalDevice);
	virtual ~Texture() = default;

	const Image& getImage() const;

	virtual void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) =0;
};