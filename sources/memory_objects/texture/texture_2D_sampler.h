#pragma once

#include "texture_2D.h"

#include <string>

class Texture2DSampler : public Texture2D {
    VkSampler _sampler          = VK_NULL_HANDLE;
    std::string _texturePath    = {};
    uint32_t _mipLevels         = 1;
    float _samplerAnisotropy    = 1.0f;

public:
    Texture2DSampler(std::shared_ptr<LogicalDevice> logicalDevice, const std::string& texturePath, float samplerAnisotropy);
    ~Texture2DSampler();

    const VkSampler getVkSampler() const;

    void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) override;
};