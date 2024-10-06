#pragma once

#include "texture.h"

class LogicalDevice;

class Texture2DShadow final : public Texture {
	const LogicalDevice& _logicalDevice;

public:
	Texture2DShadow(const LogicalDevice& logicalDevice, uint32_t width, uint32_t height, VkFormat format);
	~Texture2DShadow();
};