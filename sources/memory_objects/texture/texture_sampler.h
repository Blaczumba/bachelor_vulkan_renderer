#pragma once

#include "texture.h"

#include <vulkan/vulkan.h>

class TextureSampler : virtual public Texture {
protected:
    VkSampler _sampler          = VK_NULL_HANDLE;
    float _samplerAnisotropy    = 1.0f;

public:
    TextureSampler(float samplerAnisotropy);
    virtual ~TextureSampler() = default;

    const VkSampler getVkSampler() const;
};