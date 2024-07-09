#pragma once

#include "texture.h"

#include <string_view>

class Texture2D : public Texture {
protected:
    VkExtent2D _extent                  = {};
    VkSampleCountFlagBits _sampleCount  = VK_SAMPLE_COUNT_1_BIT;
public:
    Texture2D(std::shared_ptr<LogicalDevice> logicalDevice);
    virtual ~Texture2D() = default;
    
    const VkExtent2D& getVkExtent2D() const;
    void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) =0;
};