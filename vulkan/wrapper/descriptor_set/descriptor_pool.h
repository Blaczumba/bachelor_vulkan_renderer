#pragma once

#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

#include "descriptor_set.h"
#include "descriptor_set_layout.h"
#include "lib/buffer/buffer.h"
#include "vulkan/wrapper/logical_device/logical_device.h"

class DescriptorPool : public std::enable_shared_from_this<const DescriptorPool> {
  DescriptorPool(VkDescriptorPool descriptorPool, const LogicalDevice& logicalDevice,
                 uint32_t maxNumSets) noexcept;

public:
  ~DescriptorPool();

  static std::unique_ptr<DescriptorPool> create(
      const LogicalDevice& logicalDevice, uint32_t maxNumSets,
      VkDescriptorPoolCreateFlags flags = {});

  VkDescriptorPool getVkDescriptorPool() const;

  DescriptorSet createDesriptorSet(VkDescriptorSetLayout layout) const;

  std::vector<DescriptorSet> createDesriptorSets(
      VkDescriptorSetLayout layout, uint32_t numSets) const;

  bool maxSetsReached() const;

  const LogicalDevice& getLogicalDevice() const;

private:
  VkDescriptorPool _descriptorPool;
  const uint32_t _maxNumSets;
  mutable uint32_t _allocatedSets;

  const LogicalDevice& _logicalDevice;
};
