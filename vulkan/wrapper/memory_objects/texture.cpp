#include "texture.h"

#include <array>
#include <vma/vk_mem_alloc.h>

#include "common/util/engine_exception.h"
#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/memory_allocator/memory_allocator.h"
#include "vulkan/wrapper/memory_objects/buffer.h"
#include "vulkan/wrapper/memory_objects/buffers.h"

Texture::Texture(
    const LogicalDevice& logicalDevice, VkImage image, const Allocation allocation,
    const ImageParameters& imageParameters, VkImageLayout layout, VkSampler sampler) noexcept
  : _logicalDevice(&logicalDevice), _image(image), _allocation(allocation), _layout(layout),
    _sampler(sampler), _imageParameters(imageParameters) {}

Texture::Texture(Texture&& texture) noexcept
  : _allocation(texture._allocation), _image(std::exchange(texture._image, VK_NULL_HANDLE)),
    _views(std::move(texture._views)), _sampler(std::exchange(texture._sampler, VK_NULL_HANDLE)),
    _layout(texture._layout), _logicalDevice(texture._logicalDevice),
    _imageParameters(texture._imageParameters) {}

Texture& Texture::operator=(Texture&& texture) noexcept {
  if (this == &texture) [[unlikely]] {
    return *this;
  }

  destroy();

  _allocation = texture._allocation;
  _image = std::exchange(texture._image, VK_NULL_HANDLE);
  _views = std::move(texture._views);
  _sampler = std::exchange(texture._sampler, VK_NULL_HANDLE);
  _layout = texture._layout;
  _imageParameters = texture._imageParameters;
  _logicalDevice = texture._logicalDevice;
  return *this;
}

namespace {

struct ImageCreator {
  Allocation& allocation;
  const ImageParameters& params;

  VkImage operator()(VmaWrapper& allocator) {
    VmaWrapper::Image imageData = allocator.createVkImage(
        params, VK_IMAGE_LAYOUT_UNDEFINED, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    allocation = imageData.allocation;
    return imageData.image;
  }

  VkImage operator()(auto&&) {
    return VK_NULL_HANDLE;
  }
};

struct ImageDeleter {
  VkImage image;

  void operator()(VmaWrapper& allocator, const VmaAllocation allocation) {
    allocator.destroyVkImage(image, allocation);
  }

  void operator()(auto&&, auto&&) {}
};

}  // namespace

void Texture::destroy() {
  if (_sampler != VK_NULL_HANDLE) {
    _logicalDevice->destroyResource([sampler = _sampler](DestroyerContext context) {
      vkDestroySampler(context.device, sampler, context.allocationCallbacks);
    });
  }

  for (VkImageView view : _views) {
    _logicalDevice->destroyResource([view](DestroyerContext context) {
      vkDestroyImageView(context.device, view, context.allocationCallbacks);
    });
  }

  if (_image != VK_NULL_HANDLE) {
    _logicalDevice->destroyResource(
        [image = _image, allocation = std::move(_allocation)](DestroyerContext context) {
          std::visit(ImageDeleter{image}, *context.memoryAllocator, allocation);
        });
  }
}

Texture::~Texture() {
  destroy();
}

VkImage Texture::getVkImage() const {
  return _image;
}

VkImageView Texture::getVkImageView(size_t index) const {
  return index < _views.size() ? _views[index] : VK_NULL_HANDLE;
}

VkSampler Texture::getVkSampler() const {
  return _sampler;
}

VkExtent2D Texture::getVkExtent2D() const {
  return VkExtent2D{_imageParameters.extent.width, _imageParameters.extent.height};
}

VkExtent3D Texture::getVkExtent3D() const {
  return _imageParameters.extent;
}

VkImageLayout Texture::getVkImageLayout() const {
  return _layout;
}

namespace {

VkImageViewType getImageViewType(VkImageType type, uint32_t layerCount, VkImageCreateFlags flags) {
  switch (type) {
    case VK_IMAGE_TYPE_1D:
      {
        if (layerCount > 1) {
          return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        }
        return VK_IMAGE_VIEW_TYPE_1D;
      }
    case VK_IMAGE_TYPE_2D:
      {
        if ((flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) && layerCount == 6) {
          return VK_IMAGE_VIEW_TYPE_CUBE;
        }
        if (layerCount > 1) {
          return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        }
        return VK_IMAGE_VIEW_TYPE_2D;
      }
    case VK_IMAGE_TYPE_3D:
      return VK_IMAGE_VIEW_TYPE_3D;
  }
}

}  // namespace

VkImageView Texture::addCreateVkImageView(
    uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount) {
  if (baseMipLevel + levelCount > _imageParameters.mipLevels) [[unlikely]] {
    throw EngineException(
        "Base mip level + mip level count is greater than mip levels count of the image.");
  }

  if (baseArrayLayer + layerCount > _imageParameters.layerCount) [[unlikely]] {
    throw EngineException(
        "Base array layer + layer count is greater than layer count of the image.");
  }

  const VkImageView view = _logicalDevice->createImageView(
      _image, getImageViewType(_imageParameters.type, layerCount, _imageParameters.flags),
      _imageParameters.format, _imageParameters.aspect, baseMipLevel, levelCount, baseArrayLayer,
      layerCount);
  _views.push_back(view);
  return view;
}

void Texture::transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) {
  transitionImageLayout(commandBuffer, _image, _layout, newLayout, _imageParameters.aspect,
                        _imageParameters.mipLevels, _imageParameters.layerCount);
  _layout = newLayout;
}

namespace {

inline VkImage allocate(Allocation& allocation, const ImageParameters& imageParameters,
                        MemoryAllocator& memoryAllocator) {
  return std::visit(ImageCreator{allocation, imageParameters}, memoryAllocator);
}

}  // namespace

TextureBuilder& TextureBuilder::withType(VkImageType type) {
  _imageParameters.type = type;
  return *this;
}

TextureBuilder& TextureBuilder::withLayout(VkImageLayout layout) {
  _imageLayout = layout;
  return *this;
}

TextureBuilder& TextureBuilder::withFormat(VkFormat format) {
  _imageParameters.format = format;
  return *this;
}

TextureBuilder& TextureBuilder::withExtent(uint32_t width) {
  _imageParameters.extent = {width, 1, 1};
  return *this;
}

TextureBuilder& TextureBuilder::withExtent(uint32_t width, uint32_t height) {
  _imageParameters.extent = {width, height, 1};
  return *this;
}

TextureBuilder& TextureBuilder::withExtent(VkExtent2D extent) {
  _imageParameters.extent = {extent.width, extent.height, 1};
  return *this;
}

TextureBuilder& TextureBuilder::withExtent(uint32_t width, uint32_t height, uint32_t depth) {
  _imageParameters.extent = {width, height, depth};
  return *this;
}

TextureBuilder& TextureBuilder::withExtent(VkExtent3D extent) {
  _imageParameters.extent = extent;
  return *this;
}

TextureBuilder& TextureBuilder::withAspect(VkImageAspectFlags aspect) {
  _imageParameters.aspect = aspect;
  return *this;
}

TextureBuilder& TextureBuilder::withMipLevels(uint32_t mipLevels) {
  _imageParameters.mipLevels = mipLevels;
  return *this;
}

TextureBuilder& TextureBuilder::withNumSamples(VkSampleCountFlagBits numSamples) {
  _imageParameters.numSamples = numSamples;
  return *this;
}

TextureBuilder& TextureBuilder::withTiling(VkImageTiling tiling) {
  _imageParameters.tiling = tiling;
  return *this;
}

TextureBuilder& TextureBuilder::withUsage(VkImageUsageFlags usage) {
  _imageParameters.usage = usage;
  return *this;
}

TextureBuilder& TextureBuilder::withProperties(VkMemoryPropertyFlags properties) {
  _imageParameters.properties = properties;
  return *this;
}

TextureBuilder& TextureBuilder::withLayerCount(uint32_t layerCount) {
  _imageParameters.layerCount = layerCount;
  return *this;
}

TextureBuilder& TextureBuilder::withAdditionalCreateInfoFlags(VkImageCreateFlags flags) {
  _imageParameters.flags |= flags;
  return *this;
}

TextureBuilder& TextureBuilder::withMagFilter(VkFilter magFilter) {
  _samplerParameters.magFilter = magFilter;
  return *this;
}

TextureBuilder& TextureBuilder::withMinFilter(VkFilter minFilter) {
  _samplerParameters.minFilter = minFilter;
  return *this;
}

TextureBuilder& TextureBuilder::withMipmapMode(VkSamplerMipmapMode mipmapMode) {
  _samplerParameters.mipmapMode = mipmapMode;
  return *this;
}

TextureBuilder& TextureBuilder::withAddressModes(
    VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV,
    VkSamplerAddressMode addressModeW) {
  _samplerParameters.addressModeU = addressModeU;
  _samplerParameters.addressModeV = addressModeV;
  _samplerParameters.addressModeW = addressModeW;
  return *this;
}

TextureBuilder& TextureBuilder::withMipLodBias(float mipLodBias) {
  _samplerParameters.mipLodBias = mipLodBias;
  return *this;
}

TextureBuilder& TextureBuilder::withMaxAnisotropy(float maxAnisotropy) {
  _samplerParameters.maxAnisotropy = maxAnisotropy;
  return *this;
}

TextureBuilder& TextureBuilder::withCompareOp(VkCompareOp compareOp) {
  _samplerParameters.compareOp = compareOp;
  return *this;
}

TextureBuilder& TextureBuilder::withMinLod(float minLod) {
  _samplerParameters.minLod = minLod;
  return *this;
}

TextureBuilder& TextureBuilder::withMaxLod(float maxLod) {
  _samplerParameters.maxLod = maxLod;
  return *this;
}

TextureBuilder& TextureBuilder::withBorderColor(VkBorderColor borderColor) {
  _samplerParameters.borderColor = borderColor;
  return *this;
}

TextureBuilder& TextureBuilder::withUnnormalizedCoordinates(VkBool32 unnormalizedCoordinates) {
  _samplerParameters.unnormalizedCoordinates = unnormalizedCoordinates;
  return *this;
}

Texture TextureBuilder::buildAttachment(
    const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer) const {
  Allocation allocation;
  const VkImage image = allocate(allocation, _imageParameters, logicalDevice.getMemoryAllocator());
  transitionImageLayout(
      commandBuffer, image, VK_IMAGE_LAYOUT_UNDEFINED, _imageLayout, _imageParameters.aspect,
      _imageParameters.mipLevels, _imageParameters.layerCount);
  // TODO: remove
  const VkSampler sampler = logicalDevice.createSampler(_samplerParameters);
  return Texture(logicalDevice, image, allocation, _imageParameters, _imageLayout, sampler);
}

Texture TextureBuilder::buildImage(
    const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer, VkBuffer copyBuffer,
    const std::span<const VkBufferImageCopy> copyRegions) const {
  Allocation allocation;
  const VkImage image = allocate(allocation, _imageParameters, logicalDevice.getMemoryAllocator());
  transitionImageLayout(
      commandBuffer, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      _imageParameters.aspect, _imageParameters.mipLevels, _imageParameters.layerCount);
  copyBufferToImage(commandBuffer, copyBuffer, image, copyRegions);
  transitionImageLayout(
      commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _imageLayout,
      _imageParameters.aspect, _imageParameters.mipLevels, _imageParameters.layerCount);
  const VkSampler sampler = logicalDevice.createSampler(_samplerParameters);
  return Texture(logicalDevice, image, allocation, _imageParameters, _imageLayout, sampler);
}

Texture TextureBuilder::buildImageSampler(
    const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer) const {
  Allocation allocation;
  const VkImage image = allocate(allocation, _imageParameters, logicalDevice.getMemoryAllocator());
  transitionImageLayout(
      commandBuffer, image, VK_IMAGE_LAYOUT_UNDEFINED, _imageLayout, _imageParameters.aspect,
      _imageParameters.mipLevels, _imageParameters.layerCount);
  const VkSampler sampler = logicalDevice.createSampler(_samplerParameters);
  return Texture(logicalDevice, image, allocation, _imageParameters, _imageLayout, sampler);
}

Texture TextureBuilder::buildMipmapImage(
    const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer, VkBuffer copyBuffer,
    std::span<const VkBufferImageCopy> copyRegions) const {
  Allocation allocation;
  const VkImage image = allocate(allocation, _imageParameters, logicalDevice.getMemoryAllocator());
  transitionImageLayout(
      commandBuffer, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      _imageParameters.aspect, _imageParameters.mipLevels, _imageParameters.layerCount);
  copyBufferToImage(commandBuffer, copyBuffer, image, copyRegions);
  generateImageMipmaps(
      commandBuffer, image, _imageParameters.format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      _imageParameters.extent.width, _imageParameters.extent.height, _imageParameters.mipLevels,
      _imageParameters.layerCount);
  const VkSampler sampler = logicalDevice.createSampler(_samplerParameters);
  return Texture(logicalDevice, image, allocation, _imageParameters,
                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, sampler);
}
