#include "vulkan/wrapper/descriptor_set/lib.h"

#include <span>
#include <vulkan/vulkan.h>
#include <expected>

#include "vulkan/wrapper/util/check.h"
#include "vulkan/wrapper/descriptor_set/descriptor_pool.h"

namespace internal {

void allocateDescriptorSets(
    const DescriptorPool& descriptorPool,
    std::span<const VkDescriptorSetLayout> layouts, VkDescriptorSet* descriptorSets) {
  const VkDescriptorSetAllocateInfo allocInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .descriptorPool = descriptorPool.getVkDescriptorPool(),
    .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
    .pSetLayouts = layouts.data(),
  };

  CHECK_VKCMD(vkAllocateDescriptorSets(
                  descriptorPool.getLogicalDevice().getVkDevice(), &allocInfo, descriptorSets),
              "Failed to allocate VkDescriptorSet.");
}

}  // namespace internal
