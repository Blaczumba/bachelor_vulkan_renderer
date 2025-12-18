#pragma once

#include <span>
#include <vulkan/vulkan.h>

#include "vulkan/wrapper/logical_device/logical_device.h"

class PipelineLayout {
  PipelineLayout(const LogicalDevice& logicalDevice, VkPipelineLayout layout) noexcept;

public:
  PipelineLayout() noexcept = default;

  PipelineLayout(PipelineLayout&& other) noexcept;

  PipelineLayout& operator=(PipelineLayout&& other) noexcept;

  static PipelineLayout create(
      const LogicalDevice& logicalDevice,
      std::span<const VkDescriptorSetLayout> descriptorSetLayouts = {},
      std::span<const VkPushConstantRange> pushConstantRanges = {},
      VkPipelineLayoutCreateFlags flags = 0);

  ~PipelineLayout();

  VkPipelineLayout getVkPipelineLayout() const;

  const LogicalDevice& getLogicalDevice() const;

private:
  const LogicalDevice* _logicalDevice = nullptr;

  VkPipelineLayout _layout = VK_NULL_HANDLE;
};
