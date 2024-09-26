#pragma once

#include "texture.h"
#include <string>

class LogicalDevice;

class Texture2DImage final : public Texture {
	const std::string _texturePath;

	const LogicalDevice& _logicalDevice;

public:
	Texture2DImage(const LogicalDevice& logicalDevice, const std::string& texturePath, VkFormat format, float samplerAnisotropy);
	~Texture2DImage();
};
