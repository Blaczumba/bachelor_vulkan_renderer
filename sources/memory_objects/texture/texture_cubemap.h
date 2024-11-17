#pragma once

#include "texture.h"

#include <string>
#include <string_view>

class LogicalDevice;

class TextureCubemap final : public Texture {
	const std::string _filePath;

	const LogicalDevice& _logicalDevice;

public:
	TextureCubemap(const LogicalDevice& logicalDevice, std::string_view filePath, const Image& image, const Sampler& sampler);
	~TextureCubemap();
};
