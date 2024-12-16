#pragma once

#include "memory_allocator/memory_allocator.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <stdexcept>

struct Buffer {
    VkBuffer buffer;
    size_t size;
    void* data;
};

struct BufferDeallocator {
    const VkBuffer buffer;

    void operator()(VmaWrapper& allocator) {
        allocator.destroyVkBuffer(buffer);
    }

    void operator()(auto) {
        throw std::runtime_error("BufferDeallocator does not support such MemoryAllocator type!");
    }
};

void copyBufferToBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t layerCount);
void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, const std::vector<VkBufferImageCopy>& regions);
void copyImageToBuffer(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout layout, VkBuffer buffer, uint32_t width, uint32_t height);
void copyImageToImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImage dstImage, VkExtent2D srcSize, VkExtent2D dstSize, VkImageAspectFlagBits aspect);
void copyImageToImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImage dstImage, VkExtent2D extent, VkImageAspectFlagBits aspect);
void generateImageMipmaps(VkCommandBuffer commandBuffer, VkImage image, VkFormat imageFormat, VkImageLayout finalLayout, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, uint32_t layerCount);
