#pragma once

#include "texture_2D_sampler.h"
#include "logical_device/logical_device.h"

#include <string>

class TextureCubemap final : public Texture2DSampler {
	std::string _filePath;

public:
	TextureCubemap(std::shared_ptr<LogicalDevice> logicalDevice, std::string filePath, VkFormat format, float samplerAnisotropy);
};
