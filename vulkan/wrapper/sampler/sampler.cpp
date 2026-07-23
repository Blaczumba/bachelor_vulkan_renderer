#include "sampler.h"

#include <utility>
#include <vulkan/vulkan.h>

#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/logical_device/resource_destroyer.h"
#include "vulkan/wrapper/util/check.h"

Sampler::Sampler(const LogicalDevice& logicalDevice, VkSampler sampler) noexcept
  : _sampler(sampler), _logicalDevice(&logicalDevice) {}

Sampler Sampler::create(
    const LogicalDevice& logicalDevice, const VkSamplerCreateInfo& samplerInfo) {
  VkSampler sampler;
  CHECK_VKCMD(vkCreateSampler(logicalDevice.getVkDevice(), &samplerInfo, nullptr, &sampler),
              "Failed to create VkSampler.");
  return Sampler(logicalDevice, sampler);
}

Sampler::Sampler(Sampler&& other) noexcept
  : _sampler(std::exchange(other._sampler, VK_NULL_HANDLE)),
    _logicalDevice(std::exchange(other._logicalDevice, nullptr)) {}

Sampler& Sampler::operator=(Sampler&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  if (_sampler != VK_NULL_HANDLE) {
    destroy();
  }

  _sampler = std::exchange(other._sampler, VK_NULL_HANDLE);
  _logicalDevice = std::exchange(other._logicalDevice, nullptr);
  return *this;
}

Sampler::~Sampler() {
  if (_sampler != VK_NULL_HANDLE) {
    destroy();
    _sampler = VK_NULL_HANDLE;
  }
}

VkSampler Sampler::getVkSampler() const noexcept {
  return _sampler;
}

void Sampler::destroy() {
  _logicalDevice->destroyResource([sampler = _sampler](DestroyerContext context) {
    vkDestroySampler(context.device, sampler, context.allocationCallbacks);
  });
}

SamplerBuilder& SamplerBuilder::withMinMagFilter(VkFilter minFilter, VkFilter magFilter) noexcept {
  _minFilter = minFilter;
  _magFilter = magFilter;
  return *this;
}

SamplerBuilder& SamplerBuilder::withMipmapMode(VkSamplerMipmapMode mode) noexcept {
  _mipmapMode = mode;
  return *this;
}

SamplerBuilder& SamplerBuilder::withAddressMode(
    VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV,
    VkSamplerAddressMode addressModeW) noexcept {
  _addressModeU = addressModeU;
  _addressModeV = addressModeV;
  _addressModeW = addressModeW;
  return *this;
}

SamplerBuilder& SamplerBuilder::withMipLodBias(float mipLodBias) noexcept {
  _mipLodBias = mipLodBias;
  return *this;
}

SamplerBuilder& SamplerBuilder::withAnisotropy(float maxAnisotropy) noexcept {
  _anisotropyEnable = VK_TRUE;
  _maxAnisotropy = maxAnisotropy;
  return *this;
}

SamplerBuilder& SamplerBuilder::withCompareOp(VkCompareOp compareOp) noexcept {
  _compareEnable = VK_TRUE;
  _compareOp = compareOp;
  return *this;
}

SamplerBuilder& SamplerBuilder::withLodRange(float minLod, float maxLod) noexcept {
  _minLod = minLod;
  _maxLod = maxLod;
  _unnormalizedCoordinates = VK_FALSE;
  return *this;
}

SamplerBuilder& SamplerBuilder::withBorderColor(VkBorderColor borderColor) noexcept {
  _borderColor = borderColor;
  return *this;
}

SamplerBuilder& SamplerBuilder::withUnnormalizedCoordinates(
    VkBool32 unnormalizedCoordinates) noexcept {
  _unnormalizedCoordinates = unnormalizedCoordinates;
  return *this;
}

SamplerBuilder& SamplerBuilder::withFlags(VkSamplerCreateFlags flags) noexcept {
  _flags = flags;
  return *this;
}

SamplerMetadata SamplerBuilder::getMetadata() const noexcept {
  return SamplerMetadata{
    .flags = _flags,
    .magFilter = _magFilter,
    .minFilter = _minFilter,
    .mipmapMode = _mipmapMode,
    .addressModeU = _addressModeU,
    .addressModeV = _addressModeV,
    .addressModeW = _addressModeW,
    .mipLodBias = _mipLodBias,
    .anisotropyEnable = _anisotropyEnable,
    .maxAnisotropy = _maxAnisotropy,
    .compareEnable = _compareEnable,
    .compareOp = _compareOp,
    .minLod = _minLod,
    .maxLod = _maxLod,
    .borderColor = _borderColor,
    .unnormalizedCoordinates = _unnormalizedCoordinates,
  };
}

Sampler SamplerBuilder::build(const LogicalDevice& logicalDevice) const {
  const VkSamplerCreateInfo createInfo {
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .pNext = _pNext,
    .flags = _flags,
    .magFilter = _magFilter,
    .minFilter = _minFilter,
    .mipmapMode = _mipmapMode,
    .addressModeU = _addressModeU,
    .addressModeV = _addressModeV,
    .addressModeW = _addressModeW,
    .mipLodBias = _mipLodBias,
    .anisotropyEnable = _anisotropyEnable,
    .maxAnisotropy = _maxAnisotropy,
    .compareEnable = _compareEnable,
    .compareOp = _compareOp,
    .minLod = _minLod,
    .maxLod = _maxLod,
    .borderColor = _borderColor,
    .unnormalizedCoordinates = _unnormalizedCoordinates,
  };
  return Sampler::create(logicalDevice, createInfo);
}
