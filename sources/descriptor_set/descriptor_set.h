#pragma once

#include <logical_device/logical_device.h>
#include <memory_objects/uniform_buffer.h>

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

class DescriptorSetLayout {
	VkDescriptorSetLayout _layout;
public:
	DescriptorSetLayout();
};

class DescriptorSets {
	VkDescriptorSetLayout _descriptorSetLayout;
	VkDescriptorPool _descriptorPool;
	std::vector<VkDescriptorSet> _descriptorSets;

	std::shared_ptr<LogicalDevice> _logicalDevice;
	std::shared_ptr<DescriptorSetLayout> _layout;

public:
	DescriptorSets(std::shared_ptr<LogicalDevice> logicalDevice, const std::vector<std::vector<std::shared_ptr<UniformBufferAbstraction>>>& uniformBuffers);
	~DescriptorSets();

	VkDescriptorSetLayout getVkDescriptorSetLayout() const;
	VkDescriptorSet& getVkDescriptorSet(size_t i);

private:
	bool checkInputDataCoherence(const std::vector<std::vector<std::shared_ptr<UniformBufferAbstraction>>>& uniformBuffers) const;
};