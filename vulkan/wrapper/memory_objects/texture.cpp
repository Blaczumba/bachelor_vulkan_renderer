#include "texture.h"

#include <array>
#include <vma/vk_mem_alloc.h>

#include "common/util/engine_exception.h"
#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/memory_allocator/memory_allocator.h"
#include "vulkan/wrapper/memory_objects/buffer.h"
#include "vulkan/wrapper/memory_objects/buffers.h"

Texture::Texture(const LogicalDevice& logicalDevice, VkImage image, const Allocation allocation,
                 VkImageType type, VkFormat format, VkExtent3D extent, VkImageAspectFlags aspect,
                 VkImageCreateFlags createFlags, uint32_t mipLevels, uint32_t layerCount,
                 VkImageLayout layout, VkSampler sampler) noexcept
  : _logicalDevice(&logicalDevice), _image(image), _allocation(allocation), _imageType(type),
    _imageFormat(format), _imageExtent(extent), _imageAspect(aspect),
    _imageCreateFlags(createFlags), _mipLevels(mipLevels), _layerCount(layerCount),
    _layout(layout), _sampler(sampler) {}

Texture::Texture(Texture&& texture) noexcept
  : _allocation(texture._allocation), _image(std::exchange(texture._image, VK_NULL_HANDLE)),
    _views(std::move(texture._views)), _sampler(std::exchange(texture._sampler, VK_NULL_HANDLE)),
    _layout(texture._layout), _logicalDevice(texture._logicalDevice), _imageType(texture._imageType),
    _imageFormat(texture._imageFormat), _imageExtent(texture._imageExtent), _imageAspect(texture._imageAspect),
    _imageCreateFlags(texture._imageCreateFlags), _mipLevels(texture._mipLevels), _layerCount(texture._layerCount){}

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
  _logicalDevice = texture._logicalDevice;
  _imageType = texture._imageType;
  _imageFormat = texture._imageFormat;
  _imageExtent = texture._imageExtent;
  _imageAspect = texture._imageAspect;
  _imageCreateFlags = texture._imageCreateFlags;
  _mipLevels = texture._mipLevels;
  _layerCount = texture._layerCount;
  return *this;
}

namespace {

struct ImageCreator {
  Allocation& allocation;
  const VkImageCreateInfo& imageCreateInfo;

  VkImage operator()(VmaWrapper& allocator) {
    VmaWrapper::Image imageData = allocator.createVkImage(
        imageCreateInfo, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
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

VkImage Texture::getVkImage() const noexcept {
  return _image;
}

VkImageView Texture::getVkImageView(size_t index) const {
  return index < _views.size() ? _views[index] : VK_NULL_HANDLE;
}

VkSampler Texture::getVkSampler() const noexcept {
  return _sampler;
}

VkExtent2D Texture::getVkExtent2D() const noexcept {
  return VkExtent2D{_imageExtent.width, _imageExtent.height};
}

VkExtent3D Texture::getVkExtent3D() const noexcept {
  return _imageExtent;
}

VkImageLayout Texture::getVkImageLayout() const noexcept {
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
  if (baseMipLevel + levelCount > _mipLevels) [[unlikely]] {
    throw EngineException(
        "Base mip level + mip level count is greater than mip levels count of the image.");
  }

  if (baseArrayLayer + layerCount > _layerCount) [[unlikely]] {
    throw EngineException(
        "Base array layer + layer count is greater than layer count of the image.");
  }

  const VkImageViewCreateInfo imageViewInfo = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .image = _image,
    .viewType = getImageViewType(_imageType, layerCount, _imageCreateFlags),
    .format = _imageFormat,
    .subresourceRange = {.aspectMask = _imageAspect,
                         .baseMipLevel = baseMipLevel,
                         .levelCount = levelCount,
                         .baseArrayLayer = baseArrayLayer,
                         .layerCount = layerCount}
  };

  const VkImageView view = _logicalDevice->createImageView(imageViewInfo);
  _views.push_back(view);
  return view;
}

void Texture::transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) {
  transitionImageLayout(commandBuffer, _image, _layout, newLayout, _imageAspect,
                        _mipLevels, _layerCount);
  _layout = newLayout;
}

namespace {

inline VkImage allocate(Allocation& allocation, const VkImageCreateInfo& imageParameters,
                        MemoryAllocator& memoryAllocator) {
  return std::visit(ImageCreator{allocation, imageParameters}, memoryAllocator);
}

}  // namespace

TextureBuilder& TextureBuilder::withType(VkImageType type) noexcept {
  _imageCreateInfo.imageType = type;
  return *this;
}

TextureBuilder& TextureBuilder::withLayout(VkImageLayout layout) noexcept {
  _imageLayout = layout;
  return *this;
}

TextureBuilder& TextureBuilder::withFormat(VkFormat format) noexcept {
  _imageCreateInfo.format = format;
  return *this;
}

TextureBuilder& TextureBuilder::withExtent(uint32_t width) noexcept {
  _imageCreateInfo.extent = {width, 1, 1};
  return *this;
}

TextureBuilder& TextureBuilder::withExtent(uint32_t width, uint32_t height) noexcept {
  _imageCreateInfo.extent = {width, height, 1};
  return *this;
}

TextureBuilder& TextureBuilder::withExtent(VkExtent2D extent) noexcept {
  _imageCreateInfo.extent = {extent.width, extent.height, 1};
  return *this;
}

TextureBuilder& TextureBuilder::withExtent(
    uint32_t width, uint32_t height, uint32_t depth) noexcept {
  _imageCreateInfo.extent = {width, height, depth};
  return *this;
}

TextureBuilder& TextureBuilder::withExtent(VkExtent3D extent) noexcept {
  _imageCreateInfo.extent = extent;
  return *this;
}

TextureBuilder& TextureBuilder::withAspect(VkImageAspectFlags aspect) noexcept {
  _aspect = aspect;
  return *this;
}

TextureBuilder& TextureBuilder::withMipLevels(uint32_t mipLevels) noexcept {
  _imageCreateInfo.mipLevels = mipLevels;
  return *this;
}

TextureBuilder& TextureBuilder::withNumSamples(VkSampleCountFlagBits numSamples) noexcept {
  _imageCreateInfo.samples = numSamples;
  return *this;
}

TextureBuilder& TextureBuilder::withTiling(VkImageTiling tiling) noexcept {
  _imageCreateInfo.tiling = tiling;
  return *this;
}

TextureBuilder& TextureBuilder::withUsage(VkImageUsageFlags usage) noexcept {
  _imageCreateInfo.usage = usage;
  return *this;
}

TextureBuilder& TextureBuilder::withLayerCount(uint32_t layerCount) noexcept {
  _imageCreateInfo.arrayLayers = layerCount;
  return *this;
}

TextureBuilder& TextureBuilder::withAdditionalCreateInfoFlags(VkImageCreateFlags flags) noexcept {
  _imageCreateInfo.flags |= flags;
  return *this;
}

TextureBuilder& TextureBuilder::withMagFilter(VkFilter magFilter) noexcept {
  _samplerParameters.magFilter = magFilter;
  return *this;
}

TextureBuilder& TextureBuilder::withMinFilter(VkFilter minFilter) noexcept {
  _samplerParameters.minFilter = minFilter;
  return *this;
}

TextureBuilder& TextureBuilder::withMipmapMode(VkSamplerMipmapMode mipmapMode) noexcept {
  _samplerParameters.mipmapMode = mipmapMode;
  return *this;
}

TextureBuilder& TextureBuilder::withAddressModes(
    VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV,
    VkSamplerAddressMode addressModeW) noexcept {
  _samplerParameters.addressModeU = addressModeU;
  _samplerParameters.addressModeV = addressModeV;
  _samplerParameters.addressModeW = addressModeW;
  return *this;
}

TextureBuilder& TextureBuilder::withMipLodBias(float mipLodBias) noexcept {
  _samplerParameters.mipLodBias = mipLodBias;
  return *this;
}

TextureBuilder& TextureBuilder::withMaxAnisotropy(float maxAnisotropy) noexcept {
  _samplerParameters.maxAnisotropy = maxAnisotropy;
  return *this;
}

TextureBuilder& TextureBuilder::withCompareOp(VkCompareOp compareOp) noexcept {
  _samplerParameters.compareOp = compareOp;
  return *this;
}

TextureBuilder& TextureBuilder::withMinLod(float minLod) noexcept {
  _samplerParameters.minLod = minLod;
  return *this;
}

TextureBuilder& TextureBuilder::withMaxLod(float maxLod) noexcept {
  _samplerParameters.maxLod = maxLod;
  return *this;
}

TextureBuilder& TextureBuilder::withBorderColor(VkBorderColor borderColor) noexcept {
  _samplerParameters.borderColor = borderColor;
  return *this;
}

TextureBuilder& TextureBuilder::withUnnormalizedCoordinates(
    VkBool32 unnormalizedCoordinates) noexcept {
  _samplerParameters.unnormalizedCoordinates = unnormalizedCoordinates;
  return *this;
}

Texture TextureBuilder::buildAttachment(
    const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer) const {
  Allocation allocation;
  const VkImage image = allocate(allocation, _imageCreateInfo, logicalDevice.getMemoryAllocator());
  transitionImageLayout(
      commandBuffer, image, VK_IMAGE_LAYOUT_UNDEFINED, _imageLayout, _aspect,
      _imageCreateInfo.mipLevels, _imageCreateInfo.arrayLayers);
  // TODO: remove
  const VkSampler sampler = logicalDevice.createSampler(_samplerParameters);
  return Texture(logicalDevice, image, allocation, _imageCreateInfo.imageType, _imageCreateInfo.format,
      _imageCreateInfo.extent, _aspect, _imageCreateInfo.flags, _imageCreateInfo.mipLevels,
      _imageCreateInfo.arrayLayers, _imageLayout, sampler);
}

Texture TextureBuilder::buildImage(
    const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer, VkBuffer copyBuffer,
    const std::span<const VkBufferImageCopy> copyRegions) const {
  Allocation allocation;
  const VkImage image = allocate(allocation, _imageCreateInfo, logicalDevice.getMemoryAllocator());
  transitionImageLayout(
      commandBuffer, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      _aspect, _imageCreateInfo.mipLevels, _imageCreateInfo.arrayLayers);
  vkCmdCopyBufferToImage(commandBuffer, copyBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         static_cast<uint32_t>(copyRegions.size()), copyRegions.data());
  transitionImageLayout(
      commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _imageLayout,
      _aspect, _imageCreateInfo.mipLevels, _imageCreateInfo.arrayLayers);
  const VkSampler sampler = logicalDevice.createSampler(_samplerParameters);
  return Texture(
      logicalDevice, image, allocation, _imageCreateInfo.imageType, _imageCreateInfo.format,
      _imageCreateInfo.extent, _aspect, _imageCreateInfo.flags, _imageCreateInfo.mipLevels,
      _imageCreateInfo.arrayLayers, _imageLayout, sampler);
}

Texture TextureBuilder::buildImageSampler(
    const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer) const {
  Allocation allocation;
  const VkImage image = allocate(allocation, _imageCreateInfo, logicalDevice.getMemoryAllocator());
  transitionImageLayout(
      commandBuffer, image, VK_IMAGE_LAYOUT_UNDEFINED, _imageLayout, _aspect,
      _imageCreateInfo.mipLevels, _imageCreateInfo.arrayLayers);
  const VkSampler sampler = logicalDevice.createSampler(_samplerParameters);
  return Texture(
      logicalDevice, image, allocation, _imageCreateInfo.imageType, _imageCreateInfo.format,
      _imageCreateInfo.extent, _aspect, _imageCreateInfo.flags, _imageCreateInfo.mipLevels,
      _imageCreateInfo.arrayLayers, _imageLayout, sampler);
}

Texture TextureBuilder::buildMipmapImage(
    const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer, VkBuffer copyBuffer,
    std::span<const VkBufferImageCopy> copyRegions) const {
  Allocation allocation;
  const VkImage image = allocate(allocation, _imageCreateInfo, logicalDevice.getMemoryAllocator());
  transitionImageLayout(
      commandBuffer, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      _aspect, _imageCreateInfo.mipLevels, _imageCreateInfo.arrayLayers);
  vkCmdCopyBufferToImage(commandBuffer, copyBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         static_cast<uint32_t>(copyRegions.size()), copyRegions.data());
  generateImageMipmaps(
      commandBuffer, image, _imageCreateInfo.format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      _imageCreateInfo.extent.width, _imageCreateInfo.extent.height, _imageCreateInfo.mipLevels,
      _imageCreateInfo.arrayLayers);
  const VkSampler sampler = logicalDevice.createSampler(_samplerParameters);
  return Texture(logicalDevice, image, allocation, _imageCreateInfo.imageType,
                 _imageCreateInfo.format, _imageCreateInfo.extent, _aspect, _imageCreateInfo.flags,
                 _imageCreateInfo.mipLevels, _imageCreateInfo.arrayLayers,
                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, sampler);
}
