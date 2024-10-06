#pragma once

#include "texture.h"

#include <string>

class LogicalDevice;

class TextureCubemap final : public Texture {
	std::string _filePath;

	const LogicalDevice& _logicalDevice;

public:
	TextureCubemap(const LogicalDevice& logicalDevice, std::string filePath, VkFormat format, float samplerAnisotropy);
	~TextureCubemap();
};
