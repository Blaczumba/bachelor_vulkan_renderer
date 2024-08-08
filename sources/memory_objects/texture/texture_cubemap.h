#pragma once

#include "texture_2D_sampler.h"

#include <string>

class TextureCubemap final : public Texture2DSampler {
	std::string _filePath;

public:
	TextureCubemap(const LogicalDevice& logicalDevice, std::string filePath, VkFormat format, float samplerAnisotropy);
};
