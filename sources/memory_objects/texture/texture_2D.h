#pragma once

#include "texture.h"

#include <string_view>

class Texture2D : public Texture {
protected:
    VkImageAspectFlags _aspect          = VK_IMAGE_ASPECT_NONE;
    VkSampleCountFlagBits _sampleCount  = VK_SAMPLE_COUNT_1_BIT;
public:
    Texture2D(std::shared_ptr<LogicalDevice> logicalDevice);
    virtual ~Texture2D() = default;
    
    VkExtent2D getVkExtent2D() const;
    void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) =0;
};