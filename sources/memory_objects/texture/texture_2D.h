#pragma once

#include "texture.h"

#include <string_view>

class Texture2D : public Texture {
    VkDeviceMemory _textureImageMemory;
    VkImageView _textureImageView;
    VkSampler _textureSampler;
    VkExtent2D _extent;

    uint32_t _mipLevels;
public:
    Texture2D(std::shared_ptr<LogicalDevice> logicalDevice, std::string_view texturePath, float samplerAnisotropy = 1.0f);
    ~Texture2D();

    VkImage getVkImage() const;
    VkDeviceMemory getVkDeviceMemory() const;
    VkImageView getVkImageView() const;
    VkSampler getVkSampler() const;
    VkExtent2D getExtent() const;
};