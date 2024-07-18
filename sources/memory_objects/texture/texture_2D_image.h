#pragma once

#include "texture_2D_sampler.h"

class Texture2DImage final : public Texture2DSampler {
	std::string _texturePath = {};

public:
	Texture2DImage(std::shared_ptr<LogicalDevice> logicalDevice, const std::string& texturePath, float samplerAnisotropy);

	void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) override;
};
