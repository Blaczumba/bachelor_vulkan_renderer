#pragma once

#include "texture.h"

class LogicalDevice;

class TextureAttachment : public Texture {
    const LogicalDevice& _logicalDevice;

public:
    TextureAttachment(const LogicalDevice& logicalDevice, VkImageLayout dstLayout, const Image& image);
    ~TextureAttachment();
};