#pragma once

#include "descriptor_set.h"
#include "descriptor_set_layout.h"
#include "logical_device/logical_device.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <unordered_map>
#include <memory>
#include <stdexcept>

class DescriptorSet;	// Fix this

class DescriptorPool {
	VkDescriptorPool _descriptorPool;
	std::shared_ptr<DescriptorSetLayout> _descriptorSetLayout; // Not needed in fact but for better optimization.
	const uint32_t _maxNumSets;
	uint32_t _allocatedSets;

	std::shared_ptr<LogicalDevice> _logicalDevice;

public:
	DescriptorPool(std::shared_ptr<LogicalDevice> logicalDevice, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout, uint32_t maxNumSets);
	~DescriptorPool();

	std::unique_ptr<DescriptorSet> createDesriptorSet();

};