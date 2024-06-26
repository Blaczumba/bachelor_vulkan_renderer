#pragma once

#include "texture.h"

#include <string_view>

class Texture2D : public Texture {
    VkImage _textureImage;
    VkDeviceMemory _textureImageMemory;
    VkImageView _textureImageView;
    VkSampler _textureSampler;

    uint32_t _mipLevels;
public:
    Texture2D(std::shared_ptr<LogicalDevice> logicalDevice, std::string_view texturePath, float samplerAnisotropy = 1.0f);
    ~Texture2D();

    VkImageView getVkImageView() const;
    VkSampler getVkSampler() const;
};