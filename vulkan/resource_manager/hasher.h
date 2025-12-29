#pragma once

#include <functional>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

template <class T>
inline void hashCombine(const T& v, size_t& seed) {
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct PipelineLayoutKey {
  const std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
  const std::vector<VkPushConstantRange> pushConstants;
  const VkPipelineLayoutCreateFlags createFlags = 0;

  bool operator==(const PipelineLayoutKey& other) const {
    if (descriptorSetLayouts.size() != other.descriptorSetLayouts.size()
        || pushConstants.size() != other.pushConstants.size()) {
      return false;
    }

    return memcmp(descriptorSetLayouts.data(), other.descriptorSetLayouts.data(),
                  descriptorSetLayouts.size() * sizeof(VkDescriptorSetLayout))
               == 0
           && memcmp(pushConstants.data(), other.pushConstants.data(),
                     pushConstants.size() * sizeof(VkPushConstantRange))
                  == 0
           && createFlags == other.createFlags;
  }
};

struct PipelineLayoutHasher {
  size_t operator()(const PipelineLayoutKey& key) const {
    size_t seed = 0;
    for (VkDescriptorSetLayout layout : key.descriptorSetLayouts) {
      hashCombine(reinterpret_cast<uint64_t>(layout), seed);
    }

    for (const VkPushConstantRange& pc : key.pushConstants) {
      hashCombine(pc.size, seed);
      hashCombine(pc.offset, seed);
      hashCombine(static_cast<uint32_t>(pc.stageFlags), seed);
    }

    hashCombine(static_cast<uint32_t>(key.createFlags), seed);
    return seed;
  }
};
