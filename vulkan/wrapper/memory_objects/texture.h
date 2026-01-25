#pragma once

#include <span>
#include <vector>
#include <vulkan/vulkan.h>

#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/memory_allocator/allocation.h"
#include "vulkan/wrapper/memory_allocator/memory_allocator.h"
#include "vulkan/wrapper/memory_objects/image.h"

class Texture {
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

  VkSampler getVkSampler() const noexcept;

  VkExtent2D getVkExtent2D() const noexcept;

  VkExtent3D getVkExtent3D() const noexcept;

  VkImageLayout getVkImageLayout() const noexcept;

private:
  Texture(const LogicalDevice& logicalDevice, VkImage image, const Allocation allocation,
          const ImageParameters& imageParameters, VkImageLayout layout,
          VkSampler sampler = VK_NULL_HANDLE) noexcept;

  void destroy();

  VkImage _image = VK_NULL_HANDLE;
  std::vector<VkImageView> _views;
  // TODO: Create separate Sampler class which is not owned by Texture.
  VkSampler _sampler = VK_NULL_HANDLE;
  Allocation _allocation;
  VkImageLayout _layout;
  ImageParameters _imageParameters;

  const LogicalDevice* _logicalDevice;

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

  TextureBuilder& withProperties(VkMemoryPropertyFlags properties) noexcept;

  TextureBuilder& withLayerCount(uint32_t layerCount) noexcept;

  TextureBuilder& withAdditionalCreateInfoFlags(VkImageCreateFlags flags) noexcept;

  TextureBuilder& withMagFilter(VkFilter magFilter) noexcept;

  TextureBuilder& withMinFilter(VkFilter minFilter) noexcept;

  TextureBuilder& withMipmapMode(VkSamplerMipmapMode mipmapMode) noexcept;

  TextureBuilder& withAddressModes(
      VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV,
      VkSamplerAddressMode addressModeW) noexcept;

  TextureBuilder& withMipLodBias(float mipLodBias) noexcept;

  TextureBuilder& withMaxAnisotropy(float maxAnisotropy) noexcept;

  TextureBuilder& withCompareOp(VkCompareOp compareOp) noexcept;

  TextureBuilder& withMinLod(float minLod) noexcept;

  TextureBuilder& withMaxLod(float maxLod) noexcept;

  TextureBuilder& withBorderColor(VkBorderColor borderColor) noexcept;

  TextureBuilder& withUnnormalizedCoordinates(VkBool32 unnormalizedCoordinates) noexcept;

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
  ImageParameters _imageParameters;
  SamplerParameters _samplerParameters;
  VkImageLayout _imageLayout = VK_IMAGE_LAYOUT_GENERAL;
};
