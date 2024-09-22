#pragma once

#include "texture_2D.h"
#include "texture_sampler.h"
#include <string>

class LogicalDevice;

class Texture2DImage final : public Texture2D, public TextureSampler {
	const std::string _texturePath;

	const LogicalDevice& _logicalDevice;

public:
	Texture2DImage(const LogicalDevice& logicalDevice, const std::string& texturePath, VkFormat format, float samplerAnisotropy);
	~Texture2DImage();
};
