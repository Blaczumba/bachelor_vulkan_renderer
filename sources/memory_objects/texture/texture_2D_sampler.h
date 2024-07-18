#pragma once

#include "texture_2D.h"

#include <vulkan/vulkan.h>

class Texture2DSampler : public Texture2D {
protected:
    VkSampler _sampler          = VK_NULL_HANDLE;
    uint32_t _mipLevels         = 1;
    float _samplerAnisotropy    = 1.0f;

public:
    Texture2DSampler(std::shared_ptr<LogicalDevice> logicalDevice, float samplerAnisotropy);
    ~Texture2DSampler();

    const VkSampler getVkSampler() const;

};