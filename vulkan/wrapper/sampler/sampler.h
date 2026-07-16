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

struct SamplerMetadata {
  VkSamplerCreateFlags flags;
  VkFilter magFilter;
  VkFilter minFilter;
  VkSamplerMipmapMode mipmapMode;
  VkSamplerAddressMode addressModeU;
  VkSamplerAddressMode addressModeV;
  VkSamplerAddressMode addressModeW;
  float mipLodBias;
  VkBool32 anisotropyEnable;
  float maxAnisotropy;
  VkBool32 compareEnable;
  VkCompareOp compareOp;
  float minLod;
  float maxLod;
  VkBorderColor borderColor;
  VkBool32 unnormalizedCoordinates;
  // Other std::optional fields representing pNext metadata.
};

struct SamplerWithMetadata {
  Sampler sampler;
  SamplerMetadata metadata;
};

class SamplerBuilder {
public:
  SamplerBuilder() noexcept = default;

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

  Sampler build(const LogicalDevice& logicalDevice) const;

private:
  VkSamplerCreateInfo _samplerInfo = {
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .magFilter = VK_FILTER_NEAREST,
    .minFilter = VK_FILTER_NEAREST,
    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
    .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .minLod = 0.0f,
    .maxLod = VK_LOD_CLAMP_NONE};

  void* _pNext = nullptr;
};
