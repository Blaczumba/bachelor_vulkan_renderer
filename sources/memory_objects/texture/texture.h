#pragma once

#include "logical_device/logical_device.h"
#include "memory_objects/image.h"

#include <memory>

class Texture : public Image {
protected:
	VkSampler _sampler;
	VkImageLayout _layout;

	std::shared_ptr<LogicalDevice> _logicalDevice;
public:
	Texture(std::shared_ptr<LogicalDevice> logicalDevice, VkFormat format);
	virtual ~Texture();

	virtual void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) =0;
	VkImageLayout getVkImageLayout() const;
	VkSampler getVkSampler() const;
};