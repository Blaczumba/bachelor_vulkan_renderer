#pragma once

#include "logical_device/logical_device.h"

#include <memory>

class Texture {
protected:
	std::shared_ptr<LogicalDevice> _logicalDevice;
public:
	Texture(std::shared_ptr<LogicalDevice> logicalDevice) : _logicalDevice(logicalDevice) {}
	virtual ~Texture() = default;

	virtual VkImageView getVkImageView() const=0;
	virtual VkSampler getVkSampler() const=0;
};