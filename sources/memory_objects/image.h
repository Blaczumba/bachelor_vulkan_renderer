#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <optional>

struct Image {
    VkFormat format = VK_FORMAT_UNDEFINED;
    uint32_t width = 1;
    uint32_t height = 1;
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_NONE;
    uint32_t mipLevels = 1;
    VkSampleCountFlagBits numSamples = VK_SAMPLE_COUNT_1_BIT;
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags usage = 0;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    uint32_t layerCount = 1;
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
};

struct Sampler {
    VkFilter magFilter = VK_FILTER_LINEAR;
    VkFilter minFilter = VK_FILTER_LINEAR;
    VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    float mipLodBias = 0.0f;
    std::optional<float> maxAnisotropy = std::nullopt;
    std::optional<VkCompareOp> compareOp = std::nullopt;
    float minLod = 0.0f;
    float maxLod = 1.0f;
    VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    VkBool32 unnormalizedCoordinates = VK_FALSE;
    VkSampler sampler = VK_NULL_HANDLE;
};

void transitionImageLayout(VkCommandBuffer commandBuffer, Image* image, VkImageLayout newLayout);
void transitionLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t layerCount = 1);
void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);	// Normal texture 2D
void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, std::vector<VkBufferImageCopy>&& regions);	// Cubemap
void copyImageToBuffer(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout layout, VkBuffer buffer, uint32_t width, uint32_t height);
void copyImageToImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImage dstImage, VkExtent2D srcSize, VkExtent2D dstSize, VkImageAspectFlagBits aspect = VK_IMAGE_ASPECT_COLOR_BIT);
void copyImageToImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImage dstImage, VkExtent2D extent, VkImageAspectFlagBits aspect = VK_IMAGE_ASPECT_COLOR_BIT);
void generateImageMipmaps(VkCommandBuffer commandBuffer, Image* image, VkImageLayout finalLayout);
void generateMipmaps(VkCommandBuffer commandBuffer, VkImage image, VkFormat imageFormat, VkImageLayout finalLayout, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, uint32_t layerCount);
