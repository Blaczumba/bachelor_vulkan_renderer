#pragma once

#include "logical_device/logical_device.h"
#include "memory_objects/image.h"

#include <memory>

class Texture : public Image {
protected:
	VkImage _image;
	VkImageLayout _layout;

	std::shared_ptr<LogicalDevice> _logicalDevice;
public:
	Texture(std::shared_ptr<LogicalDevice> logicalDevice);
	virtual ~Texture() = default;

	void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) {}
	VkImageLayout getImageLayout() const;
	virtual VkImageView getVkImageView() const=0;
	virtual VkSampler getVkSampler() const=0;
};