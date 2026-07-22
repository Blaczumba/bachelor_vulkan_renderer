#pragma once

#include <span>
#include <vector>
#include <vulkan/vulkan.h>

#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/memory_allocator/allocation.h"

struct ImageMetadata {
  VkImageCreateFlags imageCreateFlags;
  VkImageType imageType;
  VkFormat imageFormat;
  VkExtent3D imageExtent;
  uint32_t mipLevels;
  uint32_t arrayLayers;
  VkSampleCountFlagBits samples;
  VkImageTiling tiling;
  VkImageUsageFlags usage;
  VkSharingMode sharingMode;
  VkImageAspectFlags imageAspect;
};

class Image {
  Image(const LogicalDevice& logicalDevice, VkImage image, const Allocation allocation) noexcept;

public:
  Image() noexcept = default;

  Image(Image&& image) noexcept;

  Image& operator=(Image&& image) noexcept;

  ~Image();

  VkImageView addCreateVkImageView(
      const ImageMetadata& metadata, uint32_t baseMipLevel, uint32_t levelCount,
      uint32_t baseArrayLayer, uint32_t layerCount);

  VkImage getVkImage() const noexcept;

  VkImageView getVkImageView(size_t index = 0) const noexcept;

  std::span<const VkImageView> getVkImageViews() const noexcept;

private:
  void destroy();

  VkImage _image = VK_NULL_HANDLE;
  Allocation _allocation;
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

  ImageMetadata getMetadata() const noexcept;

  Image buildImage(const LogicalDevice& logicalDevice, VkImageCreateFlags flags = {});

private:
  VkImageCreateFlags _imageCreateFlags = {};
  VkImageType _imageType = VK_IMAGE_TYPE_2D;
  VkFormat _format = VK_FORMAT_UNDEFINED;
  VkExtent3D _imageExtent = {1, 1, 1};
  uint32_t _mipLevels = 1;
  uint32_t _arrayLayers = 1;
  VkSampleCountFlagBits _samples = VK_SAMPLE_COUNT_1_BIT;
  VkImageTiling _tiling = VK_IMAGE_TILING_OPTIMAL;
  VkImageUsageFlags _usage = {};
  VkSharingMode _sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  VkImageAspectFlags _imageAspect = VK_IMAGE_ASPECT_COLOR_BIT;

  void* _pNext = nullptr;
};
