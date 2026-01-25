#pragma once

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

  bool isValid() const noexcept;

private:
  VkSampler _sampler = VK_NULL_HANDLE;
  const LogicalDevice* _logicalDevice = nullptr;

  void destroy();
};

class SamplerBuilder {
public:
  SamplerBuilder() noexcept = default;

  Sampler build(const LogicalDevice& logicalDevice) const;

  SamplerBuilder& setMinMagFilter(VkFilter minFiler, VkFilter magFilter) noexcept;

  SamplerBuilder& setMipmapMode(VkSamplerMipmapMode mode) noexcept;

  SamplerBuilder& setAddressMode(
	  VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV,
      VkSamplerAddressMode addressModeW) noexcept;

  SamplerBuilder& setMipLodBias(float mipLodBias) noexcept;

  SamplerBuilder& setAnisotropy(float maxAnisotropy) noexcept;

  SamplerBuilder& setCompareOp(VkCompareOp compareOp) noexcept;

  SamplerBuilder& setLodRange(float minLod, float maxLod) noexcept;

  SamplerBuilder& setBorderColor(VkBorderColor borderColor) noexcept;

  SamplerBuilder& setUnnormalizedCoordinates(VkBool32 unnormalizedCoordinates) noexcept;

  const VkSamplerCreateInfo& getVkSamplerCreateInfo() const noexcept;

private:
  VkSamplerCreateInfo _samplerInfo{};
};
