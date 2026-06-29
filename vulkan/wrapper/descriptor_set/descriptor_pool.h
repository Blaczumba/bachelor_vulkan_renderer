#pragma once

#include <memory>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

#include "descriptor_set.h"
#include "vulkan/wrapper/logical_device/logical_device.h"

class DescriptorPool : public std::enable_shared_from_this<const DescriptorPool> {
  DescriptorPool(const LogicalDevice& logicalDevice, VkDescriptorPool descriptorPool,
                 uint32_t maxNumSets, std::span<const VkDescriptorPoolSize> poolSizes) noexcept;

public:
  ~DescriptorPool();

  static std::unique_ptr<DescriptorPool> create(
      const LogicalDevice& logicalDevice, const VkDescriptorPoolCreateInfo& createInfo);

  VkDescriptorPool getVkDescriptorPool() const noexcept;

  DescriptorSet createDesriptorSet(VkDescriptorSetLayout layout) const;

  bool maxSetsReached() const noexcept;

  const LogicalDevice& getLogicalDevice() const;

private:
  VkDescriptorPool _descriptorPool;

  const uint32_t _maxNumSets;
  mutable uint32_t _allocatedSets;
  mutable lib::Buffer<VkDescriptorPoolSize> _poolSizes;

  const LogicalDevice& _logicalDevice;
};

class DescriptorPoolBuilder {
public:
  DescriptorPoolBuilder& addPoolSize(VkDescriptorType type, uint32_t descriptorCount);

  DescriptorPoolBuilder& withPoolSizes(std::span<const VkDescriptorPoolSize> poolSizes);

  DescriptorPoolBuilder& withPoolSizes(std::initializer_list<VkDescriptorPoolSize> poolSizes);

  DescriptorPoolBuilder& withPoolSizes(std::vector<VkDescriptorPoolSize>&& poolSizes) noexcept;

  std::unique_ptr<DescriptorPool> build(const LogicalDevice& logicalDevice, uint32_t maxNumSets,
                                        VkDescriptorPoolCreateFlags flags = 0);

private:
  uint32_t _maxNumSets;
  uint32_t _allocatedSets;
  std::vector<VkDescriptorPoolSize> _poolSizes = {
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000},
    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
  };
  VkDescriptorPoolCreateFlags _flags;

  void* _pNext = nullptr;
};
