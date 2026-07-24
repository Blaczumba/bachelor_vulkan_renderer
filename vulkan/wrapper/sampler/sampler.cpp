#include "sampler.h"

#include <tuple>
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

VkSamplerCreateInfo convertSamplerMetadataToVkSamplerCreateInfo(
    const SamplerMetadata& metadata) noexcept {}

SamplerBuilder&& SamplerBuilder::withMinMagFilter(
    VkFilter minFilter, VkFilter magFilter) && noexcept {
  _createInfo.minFilter = minFilter;
  _createInfo.magFilter = magFilter;
  return std::move(*this);
}

SamplerBuilder&& SamplerBuilder::withMipmapMode(VkSamplerMipmapMode mode) && noexcept {
  _createInfo.mipmapMode = mode;
  return std::move(*this);
}

SamplerBuilder&& SamplerBuilder::withAddressMode(
    VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV,
    VkSamplerAddressMode addressModeW) && noexcept {
  _createInfo.addressModeU = addressModeU;
  _createInfo.addressModeV = addressModeV;
  _createInfo.addressModeW = addressModeW;
  return std::move(*this);
}

SamplerBuilder&& SamplerBuilder::withMipLodBias(float mipLodBias) && noexcept {
  _createInfo.mipLodBias = mipLodBias;
  return std::move(*this);
}

SamplerBuilder&& SamplerBuilder::withAnisotropy(float maxAnisotropy) && noexcept {
  _createInfo.anisotropyEnable = VK_TRUE;
  _createInfo.maxAnisotropy = maxAnisotropy;
  return std::move(*this);
}

SamplerBuilder&& SamplerBuilder::withCompareOp(VkCompareOp compareOp) && noexcept {
  _createInfo.compareEnable = VK_TRUE;
  _createInfo.compareOp = compareOp;
  return std::move(*this);
}

SamplerBuilder&& SamplerBuilder::withLodRange(float minLod, float maxLod) && noexcept {
  _createInfo.minLod = minLod;
  _createInfo.maxLod = maxLod;
  _createInfo.unnormalizedCoordinates = VK_FALSE;
  return std::move(*this);
}

SamplerBuilder&& SamplerBuilder::withBorderColor(VkBorderColor borderColor) && noexcept {
  _createInfo.borderColor = borderColor;
  return std::move(*this);
}

SamplerBuilder&& SamplerBuilder::withUnnormalizedCoordinates(
    VkBool32 unnormalizedCoordinates) && noexcept {
  _createInfo.unnormalizedCoordinates = unnormalizedCoordinates;
  return std::move(*this);
}

SamplerBuilder&& SamplerBuilder::withFlags(VkSamplerCreateFlags flags) && noexcept {
  _createInfo.flags = flags;
  return std::move(*this);
}

std::tuple<Sampler, SamplerMetadata> SamplerBuilder::buildSamplerWithMetadata(
    const LogicalDevice& logicalDevice) const {
  return std::make_tuple(buildSampler(logicalDevice), buildMetadata());
}

SamplerMetadata SamplerBuilder::buildMetadata() const noexcept {
  return SamplerMetadata{
    .flags = _createInfo.flags,
    .magFilter = _createInfo.magFilter,
    .minFilter = _createInfo.minFilter,
    .mipmapMode = _createInfo.mipmapMode,
    .addressModeU = _createInfo.addressModeU,
    .addressModeV = _createInfo.addressModeV,
    .addressModeW = _createInfo.addressModeW,
    .mipLodBias = _createInfo.mipLodBias,
    .anisotropyEnable = _createInfo.anisotropyEnable,
    .maxAnisotropy = _createInfo.maxAnisotropy,
    .compareEnable = _createInfo.compareEnable,
    .compareOp = _createInfo.compareOp,
    .minLod = _createInfo.minLod,
    .maxLod = _createInfo.maxLod,
    .borderColor = _createInfo.borderColor,
    .unnormalizedCoordinates = _createInfo.unnormalizedCoordinates,
  };
}

Sampler SamplerBuilder::buildSampler(const LogicalDevice& logicalDevice) const {
  return Sampler::create(logicalDevice, _createInfo);
}
