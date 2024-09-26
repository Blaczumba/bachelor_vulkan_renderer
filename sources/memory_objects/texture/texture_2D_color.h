#pragma once

#include "texture.h"

class LogicalDevice;

class Texture2DColor : public Texture {
    const LogicalDevice& _logicalDevice;

public:
    Texture2DColor(const LogicalDevice& logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent);
    ~Texture2DColor();
};