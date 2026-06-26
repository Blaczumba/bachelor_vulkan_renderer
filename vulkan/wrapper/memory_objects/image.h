#pragma once

#include <span>
#include <vector>
#include <vulkan/vulkan.h>

#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/memory_allocator/allocation.h"
#include "vulkan/wrapper/memory_allocator/memory_allocator.h"

class Image {
  Image(const LogicalDevice& logicalDevice, VkImage image, const Allocation allocation,
        VkImageType type, VkFormat format, VkExtent3D extent, VkImageAspectFlags aspect,
        VkImageCreateFlags createFlags, uint32_t mipLevels, uint32_t arrayLevels,
        VkImageLayout layout) noexcept;

public:
  Image() noexcept = default;

  Image(Image&& image) noexcept;

  Image& operator=(Image&& image) noexcept;

  ~Image();

  VkImageView addCreateVkImageView(
      uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount);

  void generateMipmaps(VkCommandBuffer commandBuffer, VkImageLayout dstLayout);

  void copyFromBuffer(VkCommandBuffer commandBuffer, VkBuffer copyBuffer,
                      std::span<const VkBufferImageCopy> copyRegions);

  void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout);

  void transitionLayout(
      VkCommandBuffer commandBuffer, VkImageLayout newLayout, uint32_t baseMipLevel,
      uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount);

  VkImage getVkImage() const noexcept;

  VkImageView getVkImageView(size_t index = 0) const;

  VkExtent2D getVkExtent2D() const noexcept;

  VkExtent3D getVkExtent3D() const noexcept;

  uint32_t getLayersCount() const noexcept;

  uint32_t getMipLevelsCount() const noexcept;

  VkImageLayout getVkImageLayout() const noexcept;

private:
  void destroy();

  VkImage _image = VK_NULL_HANDLE;
  Allocation _allocation;
  VkImageType _imageType;
  VkFormat _imageFormat;
  VkExtent3D _imageExtent;
  VkImageAspectFlags _imageAspect;
  VkImageCreateFlags _imageCreateFlags;
  uint32_t _mipLevels;
  uint32_t _layerCount;
  VkImageLayout _layout;
  std::vector<VkImageView> _views;

  const LogicalDevice* _logicalDevice = nullptr;

  friend class ImageBuilder;
};

class ImageBuilder {
public:
  ImageBuilder& withType(VkImageType type) noexcept;

  ImageBuilder& withFormat(VkFormat format) noexcept;

  ImageBuilder& withExtent(uint32_t width) noexcept;

  ImageBuilder& withExtent(uint32_t width, uint32_t height) noexcept;

  ImageBuilder& withExtent(VkExtent2D extent) noexcept;

  ImageBuilder& withExtent(uint32_t width, uint32_t height, uint32_t depth) noexcept;

  ImageBuilder& withExtent(VkExtent3D extent) noexcept;

  ImageBuilder& withAspect(VkImageAspectFlags aspect) noexcept;

  ImageBuilder& withMipLevels(uint32_t mipLevels) noexcept;

  ImageBuilder& withNumSamples(VkSampleCountFlagBits numSamples) noexcept;

  ImageBuilder& withTiling(VkImageTiling tiling) noexcept;

  ImageBuilder& withUsage(VkImageUsageFlags usage) noexcept;

  ImageBuilder& withLayerCount(uint32_t layerCount) noexcept;

  ImageBuilder& withAdditionalCreateInfoFlags(VkImageCreateFlags flags) noexcept;

  Image buildImage(const LogicalDevice& logicalDevice) const;

private:
  VkImageCreateInfo _imageCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = VK_FORMAT_UNDEFINED,
    .extent = {1, 1, 1},
    .mipLevels = 1,
    .arrayLayers = 1,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .usage = 0,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
  };
  VkImageAspectFlags _aspect = VK_IMAGE_ASPECT_COLOR_BIT;
};
