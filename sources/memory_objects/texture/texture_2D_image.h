#pragma once

#include "texture_2D_sampler.h"
#include <string>

class Texture2DImage final : public Texture2DSampler {
	const std::string _texturePath;

public:
	Texture2DImage(const LogicalDevice& logicalDevice, const std::string& texturePath, VkFormat format, float samplerAnisotropy);
};
