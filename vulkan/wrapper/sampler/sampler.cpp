#include "sampler.h"

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

Sampler SamplerBuilder::build(const LogicalDevice& logicalDevice) const {
  return Sampler::create(logicalDevice, _samplerInfo);
}

SamplerBuilder& SamplerBuilder::withMinMagFilter(VkFilter minFilter, VkFilter magFilter) noexcept {
  _samplerInfo.minFilter = minFilter;
  _samplerInfo.magFilter = magFilter;
  return *this;
}

SamplerBuilder& SamplerBuilder::withMipmapMode(VkSamplerMipmapMode mode) noexcept {
  _samplerInfo.mipmapMode = mode;
  return *this;
}

SamplerBuilder& SamplerBuilder::withAddressMode(
    VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV,
    VkSamplerAddressMode addressModeW) noexcept {
  _samplerInfo.addressModeU = addressModeU;
  _samplerInfo.addressModeV = addressModeV;
  _samplerInfo.addressModeW = addressModeW;
  return *this;
}

SamplerBuilder& SamplerBuilder::withMipLodBias(float mipLodBias) noexcept {
  _samplerInfo.mipLodBias = mipLodBias;
  return *this;
}

SamplerBuilder& SamplerBuilder::withAnisotropy(float maxAnisotropy) noexcept {
  _samplerInfo.anisotropyEnable = VK_TRUE;
  _samplerInfo.maxAnisotropy = maxAnisotropy;
  return *this;
}

SamplerBuilder& SamplerBuilder::withCompareOp(VkCompareOp compareOp) noexcept {
  _samplerInfo.compareEnable = VK_TRUE;
  _samplerInfo.compareOp = compareOp;
  return *this;
}

SamplerBuilder& SamplerBuilder::withLodRange(float minLod, float maxLod) noexcept {
  _samplerInfo.minLod = minLod;
  _samplerInfo.maxLod = maxLod;
  _samplerInfo.unnormalizedCoordinates = VK_FALSE;
  return *this;
}

SamplerBuilder& SamplerBuilder::withBorderColor(VkBorderColor borderColor) noexcept {
  _samplerInfo.borderColor = borderColor;
  return *this;
}

SamplerBuilder& SamplerBuilder::withUnnormalizedCoordinates(
    VkBool32 unnormalizedCoordinates) noexcept {
  _samplerInfo.unnormalizedCoordinates = unnormalizedCoordinates;
  return *this;
}

const VkSamplerCreateInfo& SamplerBuilder::getVkSamplerCreateInfo() const noexcept {
  return _samplerInfo;
}
