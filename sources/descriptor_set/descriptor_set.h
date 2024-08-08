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
	std::vector<std::vector<uint32_t>> _offsets;

	const LogicalDevice& _logicalDevice;

public:
	DescriptorSet(const LogicalDevice& logicalDevice, const DescriptorPool& descriptorPool);
	~DescriptorSet();

	void updateDescriptorSet(const std::vector<UniformBuffer*>& uniformBuffers, uint32_t dynamicOffsetCount = 1);
	void bindDescriptorSet(VkCommandBuffer commandBuffer, const Pipeline& pipeline, uint32_t dynamicOffsetStride = 0);

	const VkDescriptorSet getVkDescriptorSet(size_t i) const;

};