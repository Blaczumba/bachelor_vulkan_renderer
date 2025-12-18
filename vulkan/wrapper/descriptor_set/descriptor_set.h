#pragma once

#include <memory>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

#include "lib/buffer/buffer.h"
#include "vulkan/wrapper/descriptor_set/descriptor_set_layout.h"

class DescriptorPool;  // DescriptorPool is forward declared to avoid circular dependency

class DescriptorSet {
  DescriptorSet(VkDescriptorSet descriptorSet,
                const std::shared_ptr<const DescriptorPool>& descriptorPool) noexcept;

public:
  DescriptorSet() noexcept = default;

  DescriptorSet(DescriptorSet&& descriptorSet) noexcept;

  DescriptorSet& operator=(DescriptorSet&& DescriptorSet) noexcept;

  ~DescriptorSet() = default;

  static DescriptorSet create(
      const std::shared_ptr<const DescriptorPool>& descriptorPool, VkDescriptorSetLayout layout);

  static std::vector<DescriptorSet> create(
      const std::shared_ptr<const DescriptorPool>& descriptorPool, VkDescriptorSetLayout layout,
      uint32_t numSets);

  VkDescriptorSet getVkDescriptorSet() const;

  const DescriptorPool& getDescriptorPool() const;

private:
  VkDescriptorSet _descriptorSet = VK_NULL_HANDLE;

  std::shared_ptr<const DescriptorPool> _descriptorPool;
};
