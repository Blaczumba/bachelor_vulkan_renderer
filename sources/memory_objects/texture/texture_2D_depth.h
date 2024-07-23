#pragma once

#include "texture_2D.h"

class Texture2DDepth : public Texture2D {
public:
	Texture2DDepth(std::shared_ptr<LogicalDevice> logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent);
    ~Texture2DDepth();

private:
	bool hasStencil(VkFormat format) const;
	// bool hasDepth(VkFormat format) const;
};