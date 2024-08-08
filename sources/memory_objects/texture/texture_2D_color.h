#pragma once

#include "texture_2D.h"

class Texture2DColor : public Texture2D {
public:
    Texture2DColor(const LogicalDevice& logicalDevice, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent);
    ~Texture2DColor();
};