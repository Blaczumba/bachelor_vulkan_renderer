#pragma once

#include <span>
#include <vulkan/vulkan.h>

void copyBufferToBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                        VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize size);

void transitionImageLayout(
    VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
    VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t layerCount);

void generateImageMipmaps(
    VkCommandBuffer commandBuffer, VkImage image, VkFormat imageFormat, VkImageLayout finalLayout,
    int32_t texWidth, int32_t texHeight, uint32_t mipLevels, uint32_t layerCount);
