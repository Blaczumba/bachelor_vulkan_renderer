#pragma once

#include "texture.h"

#include <string_view>

class Texture2D : virtual public Texture {
protected:
    VkSampleCountFlagBits _sampleCount;

public:
    Texture2D(VkSampleCountFlagBits sampleCount);
    virtual ~Texture2D() = default;
    
    VkExtent2D getVkExtent2D() const;
};