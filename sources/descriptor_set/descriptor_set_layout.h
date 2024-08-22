#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <unordered_map>

using DescriptorTypeCounterDict = std::unordered_map<VkDescriptorType, uint8_t>;

class LogicalDevice;

class DescriptorSetLayout {
	VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSetLayoutBinding> _bindings;
	DescriptorTypeCounterDict _descriptorTypeOccurances;
	uint32_t _binding;

	const LogicalDevice& _logicalDevice;

public:
	DescriptorSetLayout(const LogicalDevice& logicalDevice);
	~DescriptorSetLayout();

	void addLayoutBinding(VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1, const VkSampler* pImmutableSamplers = nullptr);
	void create();

	const VkDescriptorSetLayout getVkDescriptorSetLayout() const;
	const DescriptorTypeCounterDict& getDescriptorTypeCounter() const;
};