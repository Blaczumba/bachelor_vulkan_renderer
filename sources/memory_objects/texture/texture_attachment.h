#pragma once

#include "texture.h"

class CommandPool;
class LogicalDevice;

class TextureAttachment : public Texture {
    const LogicalDevice& _logicalDevice;

public:
    TextureAttachment(const CommandPool& commandPool, VkImageLayout dstLayout, const Image& image);
    ~TextureAttachment();
};