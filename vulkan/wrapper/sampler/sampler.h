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

  SamplerBuilder& withFlags(VkSamplerCreateFlags flags) noexcept;

  SamplerMetadata getMetadata() const noexcept;

  Sampler build(const LogicalDevice& logicalDevice) const;

private:
  VkSamplerCreateFlags _flags = {};
  VkFilter _magFilter = VK_FILTER_LINEAR;
  VkFilter _minFilter = VK_FILTER_LINEAR;
  VkSamplerMipmapMode _mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  VkSamplerAddressMode _addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  VkSamplerAddressMode _addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  VkSamplerAddressMode _addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  float _mipLodBias = 0.0f;
  VkBool32 _anisotropyEnable = VK_FALSE;
  float _maxAnisotropy = 0.0f;
  VkBool32 _compareEnable = VK_FALSE;
  VkCompareOp _compareOp = VK_COMPARE_OP_NEVER;
  float _minLod = 0.0f;
  float _maxLod = VK_LOD_CLAMP_NONE;
  VkBorderColor _borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
  VkBool32 _unnormalizedCoordinates = VK_FALSE;

  void* _pNext = nullptr;
};
