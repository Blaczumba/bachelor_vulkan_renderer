#pragma once

#include "texture.h"

#include <string_view>

class Texture2D : public Texture {
protected:
    VkSampleCountFlagBits _sampleCount;

public:
    Texture2D(std::shared_ptr<LogicalDevice> logicalDevice);
    virtual ~Texture2D() = default;
    
    VkExtent2D getVkExtent2D() const;
};