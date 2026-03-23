#pragma once

#include <vulkan/vulkan.h>

#include "vulkan/wrapper/logical_device/logical_device.h"

class Sampler {
  Sampler(const LogicalDevice& logicalDevice, VkSampler sampler) noexcept;

public:
  Sampler() noexcept = default;

  static Sampler create(const LogicalDevice& logicalDevice, const VkSamplerCreateInfo& samplerInfo);

  Sampler(Sampler&& other) noexcept;

  Sampler& operator=(Sampler&& other) noexcept;

  ~Sampler();

  VkSampler getVkSampler() const noexcept;

private:
  VkSampler _sampler = VK_NULL_HANDLE;
  const LogicalDevice* _logicalDevice = nullptr;

  void destroy();
};

class SamplerBuilder {
public:
  SamplerBuilder() noexcept = default;

  Sampler build(const LogicalDevice& logicalDevice) const;

  SamplerBuilder& withMinMagFilter(VkFilter minFiler, VkFilter magFilter) noexcept;

  SamplerBuilder& withMipmapMode(VkSamplerMipmapMode mode) noexcept;

  SamplerBuilder& withAddressMode(
      VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV,
      VkSamplerAddressMode addressModeW) noexcept;

  SamplerBuilder& withMipLodBias(float mipLodBias) noexcept;

  SamplerBuilder& withAnisotropy(float maxAnisotropy) noexcept;

  SamplerBuilder& withCompareOp(VkCompareOp compareOp) noexcept;

  SamplerBuilder& withLodRange(float minLod, float maxLod) noexcept;

  SamplerBuilder& withBorderColor(VkBorderColor borderColor) noexcept;

  SamplerBuilder& withUnnormalizedCoordinates(VkBool32 unnormalizedCoordinates) noexcept;

  const VkSamplerCreateInfo& getVkSamplerCreateInfo() const noexcept;

private:
  VkSamplerCreateInfo _samplerInfo = {
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .magFilter = VK_FILTER_LINEAR,
    .minFilter = VK_FILTER_LINEAR,
    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
    .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .minLod = 0.0f,
    .maxLod = VK_LOD_CLAMP_NONE,
    .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
    .unnormalizedCoordinates = VK_FALSE};
};
