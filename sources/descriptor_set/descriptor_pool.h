#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <unordered_map>
#include <memory>
#include <stdexcept>

class LogicalDevice;
class DescriptorSetLayout;
class DescriptorSet;

class DescriptorPool : public std::enable_shared_from_this<const DescriptorPool> {
	VkDescriptorPool _descriptorPool;
	const uint32_t _maxNumSets;
	mutable uint32_t _allocatedSets;

	const LogicalDevice& _logicalDevice;
	const DescriptorSetLayout& _descriptorSetLayout;

public:
	DescriptorPool(const LogicalDevice& logicalDevice, const DescriptorSetLayout& descriptorSetLayout, uint32_t maxNumSets);
	~DescriptorPool();

	const VkDescriptorPool getVkDescriptorPool() const;
	const DescriptorSetLayout& getDescriptorSetLayout() const;
	std::unique_ptr<DescriptorSet> createDesriptorSet() const;
	bool maxSetsReached() const;

};