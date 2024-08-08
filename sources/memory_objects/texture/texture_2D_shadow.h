#pragma once

#include "texture_2D_sampler.h"

class Texture2DShadow final : public Texture2DSampler {
public:
	Texture2DShadow(const LogicalDevice& logicalDevice, uint32_t width, uint32_t height, VkFormat depthFormat);
};