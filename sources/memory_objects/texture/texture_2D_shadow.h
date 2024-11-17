#pragma once

#include "texture.h"

class LogicalDevice;

class Texture2DShadow final : public Texture {
	const LogicalDevice& _logicalDevice;

public:
	Texture2DShadow(const LogicalDevice& logicalDevice, const Image& image, const Sampler& sampler);
	~Texture2DShadow();
};