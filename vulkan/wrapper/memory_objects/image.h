#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

struct SamplerParameters {
  VkFilter magFilter = VK_FILTER_LINEAR;
  VkFilter minFilter = VK_FILTER_LINEAR;
  VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  float mipLodBias = 0.0f;
  std::optional<float> maxAnisotropy = std::nullopt;
  std::optional<VkCompareOp> compareOp = std::nullopt;
  float minLod = 0.0f;
  float maxLod = 1.0f;
  VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  VkBool32 unnormalizedCoordinates = VK_FALSE;
};
