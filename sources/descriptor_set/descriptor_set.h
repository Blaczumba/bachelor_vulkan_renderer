#pragma once

#include "logical_device/logical_device.h"
#include "pipeline/pipeline.h"
#include "memory_objects/uniform_buffer/uniform_buffer.h"
#include "descriptor_pool.h"
#include "descriptor_set_layout.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

class DescriptorSet {
	VkDescriptorSet _descriptorSet;
	std::shared_ptr<DescriptorSetLayout> _descriptorSetLayout;

	std::vector<uint32_t> _dynamicBuffersBaseSizes;
	std::vector<std::vector<uint32_t>> _offsets;

	std::shared_ptr<LogicalDevice> _logicalDevice;

public:
	DescriptorSet(std::shared_ptr<LogicalDevice> logicalDevice, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout, VkDescriptorPool descriptorPool);
	~DescriptorSet();

	void updateDescriptorSet(const std::vector<std::shared_ptr<UniformBuffer>>& uniformBuffers, uint32_t dynamicOffsetCount = 1);
	void bindDescriptorSet(VkCommandBuffer commandBuffer, const Pipeline& pipeline, uint32_t dynamicOffsetStride = 0);

	VkDescriptorSetLayout getVkDescriptorSetLayout() const;
	VkDescriptorSet& getVkDescriptorSet(size_t i);

};