#pragma once

#include <vulkan/vulkan.h>
#include <span>
#include <expected>

#include "lib/buffer/buffer.h"

class DescriptorPool;

namespace internal {

void allocateDescriptorSets(
    const DescriptorPool& descriptorPool,
    std::span<const VkDescriptorSetLayout> layouts, VkDescriptorSet* descriptorSets);

}  // namespace internal
