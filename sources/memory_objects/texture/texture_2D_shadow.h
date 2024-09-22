#pragma once

#include "texture_2D.h"
#include "texture_sampler.h"

class LogicalDevice;

class Texture2DShadow final : public Texture2D, public TextureSampler {
	const LogicalDevice& _logicalDevice;

public:
	Texture2DShadow(const LogicalDevice& logicalDevice, uint32_t width, uint32_t height, VkFormat depthFormat);
	~Texture2DShadow();
};