#pragma once

#include "descriptor_set.h"

#include <vulkan/vulkan.h>

#include <vector>

// TODO multiple descriptor sets management
class DescriptorSetsAggregator {
	std::vector<VkDescriptorSet> _descriptorSets;
public:
	void changeDescriptorSetsCount(size_t count);
	void modifyDescriptorSetCollection(uint32_t index, const DescriptorSet& descriptorSet);
};