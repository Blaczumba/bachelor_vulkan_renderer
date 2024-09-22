#pragma once

#include "texture_2D.h"
#include "texture_sampler.h"

#include <string>

class LogicalDevice;

class TextureCubemap final : public Texture2D, public TextureSampler {
	std::string _filePath;

	const LogicalDevice& _logicalDevice;

public:
	TextureCubemap(const LogicalDevice& logicalDevice, std::string filePath, VkFormat format, float samplerAnisotropy);
	~TextureCubemap();
};
