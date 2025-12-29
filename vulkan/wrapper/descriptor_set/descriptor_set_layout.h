#pragma once

#include <span>
#include <vulkan/vulkan.h>

#include "vulkan/wrapper/logical_device/logical_device.h"

class DescriptorSetLayout {
  DescriptorSetLayout(const LogicalDevice& logicalDevice, VkDescriptorSetLayout layout) noexcept;

public:
  DescriptorSetLayout() noexcept = default;

  DescriptorSetLayout(DescriptorSetLayout&& layout) noexcept;

  DescriptorSetLayout& operator=(DescriptorSetLayout&& layout) noexcept;

  ~DescriptorSetLayout();

  static DescriptorSetLayout create(
      const LogicalDevice& logicalDevice, std::span<const VkDescriptorSetLayoutBinding> bindings,
      std::span<const VkDescriptorBindingFlags> bindingFlags = {},
      VkDescriptorSetLayoutCreateFlags flags = 0);

  VkDescriptorSetLayout getVkDescriptorSetLayout() const;

private:
  void destroy();

  VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;

  const LogicalDevice* _logicalDevice;
};
