#pragma once

#include "texture.h"

#include <string_view>

class Texture2D : public Texture {
    uint32_t _mipLevels;
public:
    Texture2D(std::shared_ptr<LogicalDevice> logicalDevice, std::string_view texturePath, float samplerAnisotropy = 1.0f);

    void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) override;
};