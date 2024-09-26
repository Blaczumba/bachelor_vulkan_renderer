#pragma once

#include "texture.h"

class LogicalDevice;

class Texture2DDepth : public Texture {
	const LogicalDevice& _logicalDevice;

public:
	Texture2DDepth(const LogicalDevice& logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent);
    ~Texture2DDepth();

private:
	bool hasStencil(VkFormat format) const;
	// bool hasDepth(VkFormat format) const;
};