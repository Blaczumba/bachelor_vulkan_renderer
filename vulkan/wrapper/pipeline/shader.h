#pragma once

#include <span>
#include <vulkan/vulkan.h>

#include "lib/buffer/buffer.h"

class LogicalDevice;

class Shader {
  Shader(VkShaderModule shaderModule, const LogicalDevice& logicalDevice,
         VkShaderStageFlagBits shaderStage) noexcept;

public:
  Shader() noexcept = default;

  static Shader create(const LogicalDevice& logicalDevice, std::span<const std::byte> shaderData,
                       VkShaderStageFlagBits shaderStage);

  ~Shader();

  Shader(Shader&& other) noexcept;

  Shader& operator=(Shader&& other) noexcept;

  VkPipelineShaderStageCreateInfo getVkPipelineStageCreateInfo() const noexcept;

  VkShaderStageFlagBits getVkShaderStageFlagBits() const noexcept;

private:
  void destroy();

  VkShaderModule _shaderModule = VK_NULL_HANDLE;
  VkShaderStageFlagBits _shaderStage;
  const LogicalDevice* _logicalDevice = nullptr;
};
