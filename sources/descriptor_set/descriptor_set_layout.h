#pragma once

#include "logical_device/logical_device.h"

#include <vulkan/vulkan.h>

#include <unordered_map>

using DescriptorTypeCounterDict = std::unordered_map<VkDescriptorType, uint8_t>;

class DescriptorSetLayout {
	VkDescriptorSetLayout _descriptorSetLayout = nullptr;
	std::vector<VkDescriptorSetLayoutBinding> _bindings;
	DescriptorTypeCounterDict _descriptorTypeOccurances;
	uint32_t _binding;

	std::shared_ptr<LogicalDevice> _logicalDevice;

public:
	DescriptorSetLayout(std::shared_ptr<LogicalDevice> logicalDevice);
	~DescriptorSetLayout();

	void addLayoutBinding(VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1, const VkSampler* pImmutableSamplers = nullptr);
	void create();

	const VkDescriptorSetLayout getVkDescriptorSetLayout() const;
	const DescriptorTypeCounterDict& getDescriptorTypeCounter() const;
};