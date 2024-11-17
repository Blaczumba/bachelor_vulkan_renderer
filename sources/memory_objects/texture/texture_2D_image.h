#pragma once

#include "texture.h"

#include <string>

class CommandPool;
class LogicalDevice;

class Texture2DImage final : public Texture {
	const std::string _texturePath;

	const LogicalDevice& _logicalDevice;

public:
	Texture2DImage(const CommandPool& commandPool, std::string_view texturePath, const Image& image, const Sampler& sampler);
	~Texture2DImage();
};
