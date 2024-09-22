#pragma once

#include "texture_2D.h"

class LogicalDevice;

class Texture2DDepth : public Texture2D {
	const LogicalDevice& _logicalDevice;

public:
	Texture2DDepth(const LogicalDevice& logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent);
    ~Texture2DDepth();

private:
	bool hasStencil(VkFormat format) const;
	// bool hasDepth(VkFormat format) const;
};