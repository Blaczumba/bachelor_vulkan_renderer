#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

class DescriptorPool;
class LogicalDevice;
class UniformBuffer;
class Pipeline;

class DescriptorSet {
	VkDescriptorSet _descriptorSet;

	std::vector<uint32_t> _dynamicBuffersBaseSizes;

	const LogicalDevice& _logicalDevice;

public:
	DescriptorSet(const LogicalDevice& logicalDevice, const DescriptorPool& descriptorPool);
	~DescriptorSet();

	void updateDescriptorSet(const std::vector<UniformBuffer*>& uniformBuffers);
	void bindDescriptorSet(VkCommandBuffer commandBuffer, const Pipeline& pipeline, const std::vector<uint32_t>& dynamicOffsetStrides = {});

	const VkDescriptorSet getVkDescriptorSet(size_t i) const;

};