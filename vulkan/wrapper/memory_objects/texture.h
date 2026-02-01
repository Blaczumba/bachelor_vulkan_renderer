#pragma once

#include <span>
#include <vector>
#include <vulkan/vulkan.h>

#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/memory_allocator/allocation.h"
#include "vulkan/wrapper/memory_allocator/memory_allocator.h"

class Texture {
  Texture(const LogicalDevice& logicalDevice, VkImage image, const Allocation allocation,
          VkImageType type, VkFormat format, VkExtent3D extent, VkImageAspectFlags aspect,
          VkImageCreateFlags createFlags, uint32_t mipLevels, uint32_t arrayLevels,
          VkImageLayout layout) noexcept;

public:
  Texture() noexcept = default;

  Texture(Texture&& texture) noexcept;

  Texture& operator=(Texture&& texuture) noexcept;

  ~Texture();

  VkImageView addCreateVkImageView(
      uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount);

  void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout);

  VkImage getVkImage() const noexcept;

  VkImageView getVkImageView(size_t index = 0) const;

  VkExtent2D getVkExtent2D() const noexcept;

  VkExtent3D getVkExtent3D() const noexcept;

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

  friend class TextureBuilder;
};

class TextureBuilder {
public:
  TextureBuilder& withType(VkImageType type) noexcept;

  TextureBuilder& withLayout(VkImageLayout layout) noexcept;

  TextureBuilder& withFormat(VkFormat format) noexcept;

  TextureBuilder& withExtent(uint32_t width) noexcept;

  TextureBuilder& withExtent(uint32_t width, uint32_t height) noexcept;

  TextureBuilder& withExtent(VkExtent2D extent) noexcept;

  TextureBuilder& withExtent(uint32_t width, uint32_t height, uint32_t depth) noexcept;

  TextureBuilder& withExtent(VkExtent3D extent) noexcept;

  TextureBuilder& withAspect(VkImageAspectFlags aspect) noexcept;

  TextureBuilder& withMipLevels(uint32_t mipLevels) noexcept;

  TextureBuilder& withNumSamples(VkSampleCountFlagBits numSamples) noexcept;

  TextureBuilder& withTiling(VkImageTiling tiling) noexcept;

  TextureBuilder& withUsage(VkImageUsageFlags usage) noexcept;

  TextureBuilder& withLayerCount(uint32_t layerCount) noexcept;

  TextureBuilder& withAdditionalCreateInfoFlags(VkImageCreateFlags flags) noexcept;

  Texture buildAttachment(const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer) const;

  Texture buildImage(
      const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer, VkBuffer copyBuffer,
      const std::span<const VkBufferImageCopy> copyRegions) const;

  Texture buildImageSampler(
      const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer) const;

  Texture buildMipmapImage(
      const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer, VkBuffer copyBuffer,
      std::span<const VkBufferImageCopy> copyRegions) const;

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
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};
  VkImageAspectFlags _aspect = VK_IMAGE_ASPECT_COLOR_BIT;
  VkImageLayout _imageLayout = VK_IMAGE_LAYOUT_GENERAL;
};
