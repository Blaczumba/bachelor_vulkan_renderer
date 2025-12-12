#pragma once

#include <span>
#include <vulkan/vulkan.h>

#include "common/status/status.h"
#include "vulkan/wrapper/logical_device/logical_device.h"

class PipelineLayout {
  PipelineLayout(const LogicalDevice& logicalDevice, VkPipelineLayout layout);

public:
  PipelineLayout() = default;

  PipelineLayout(PipelineLayout&& other) noexcept;

  PipelineLayout& operator=(PipelineLayout&& other) noexcept;

  static ErrorOr<PipelineLayout> create(
      const LogicalDevice& logicalDevice,
      std::span<const VkDescriptorSetLayout> descriptorSetLayouts = {},
      std::span<const VkPushConstantRange> pushConstantRanges = {},
      VkPipelineLayoutCreateFlags flags = 0);

  static ErrorOr<PipelineLayout> wrap(const LogicalDevice& logicalDevice, VkPipelineLayout layout);

  ~PipelineLayout();

  VkPipelineLayout getVkPipelineLayout() const;

  const LogicalDevice& getLogicalDevice() const;

private:
  const LogicalDevice* _logicalDevice = nullptr;

  VkPipelineLayout _layout = VK_NULL_HANDLE;
};
