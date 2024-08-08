#pragma once

#include "texture_2D.h"

#include <vulkan/vulkan.h>

class Texture2DSampler : public Texture2D {
protected:
    VkSampler _sampler;
    float _samplerAnisotropy;

public:
    Texture2DSampler(const LogicalDevice& logicalDevice, float samplerAnisotropy);
    ~Texture2DSampler();

    const VkSampler getVkSampler() const;

protected:
    // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL -> VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    // This can be changed in the future.
    void generateMipmaps(VkCommandBuffer commandBuffer, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
};