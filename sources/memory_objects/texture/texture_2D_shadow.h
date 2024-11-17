#pragma once

#include "texture.h"

class CommandPool;
class LogicalDevice;

class Texture2DShadow final : public Texture {
	const LogicalDevice& _logicalDevice;

public:
	Texture2DShadow(const CommandPool& commandPool, const Image& image, const Sampler& sampler);
	~Texture2DShadow();
};