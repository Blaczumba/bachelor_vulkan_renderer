#pragma once

#include <cstdint>
#include <span>
#include <utility>
#include <vector>
#include <vulkan/vulkan.h>

#include "lib/buffer/buffer.h"
#include "vulkan/wrapper/logical_device/logical_device.h"

class DescriptorSetLayout {
  DescriptorSetLayout(const LogicalDevice& logicalDevice, VkDescriptorSetLayout layout) noexcept;

public:
  DescriptorSetLayout() noexcept = default;

  DescriptorSetLayout(DescriptorSetLayout&& layout) noexcept;

  DescriptorSetLayout& operator=(DescriptorSetLayout&& layout) noexcept;

  ~DescriptorSetLayout();

  static DescriptorSetLayout create(
      const LogicalDevice& logicalDevice, const VkDescriptorSetLayoutCreateInfo& createInfo);

  VkDescriptorSetLayout getVkDescriptorSetLayout() const noexcept;

private:
  void destroy();

  VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;

  const LogicalDevice* _logicalDevice;
};

struct DescriptorSetLayoutMetadata {
  lib::Buffer<std::pair<VkDescriptorSetLayoutBinding, VkDescriptorBindingFlags>> bindings;
  VkDescriptorSetLayoutCreateFlags flags;
  // Other std::optional fields representing pNext metadata.
};

class DescriptorSetLayoutBuilder {
public:
  DescriptorSetLayoutBuilder& addBinding(
      uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount,
      VkShaderStageFlags stageFlags, VkDescriptorBindingFlags bindingFlags = 0,
      const VkSampler* immutableSamplers = nullptr);

  DescriptorSetLayoutMetadata getMetadata() const;

  DescriptorSetLayout build(
      const LogicalDevice& logicalDevice, VkDescriptorSetLayoutCreateFlags flags = 0);

private:
  std::vector<VkDescriptorSetLayoutBinding> _bindings;
  std::vector<VkDescriptorBindingFlags> _bindingFlags;
  VkDescriptorSetLayoutCreateFlags _flags;
};
