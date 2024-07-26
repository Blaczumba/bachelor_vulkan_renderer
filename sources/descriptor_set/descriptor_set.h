#pragma once

#include "logical_device/logical_device.h"
#include "pipeline/pipeline.h"
#include "memory_objects/uniform_buffer/uniform_buffer.h"
#include "descriptor_pool.h"
#include "descriptor_set_layout.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <array>
#include <memory>

class DescriptorSet {
	VkDescriptorSet _descriptorSet;
	std::shared_ptr<DescriptorSetLayout> _descriptorSetLayout;

	std::vector<uint32_t> _dynamicBuffersBaseSizes;
	std::array<uint32_t, 100> _offsets;

	std::shared_ptr<LogicalDevice> _logicalDevice;

public:
	DescriptorSet(std::shared_ptr<LogicalDevice> logicalDevice, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout, VkDescriptorPool descriptorPool);
	~DescriptorSet();

	void updateDescriptorSet(const std::vector<std::shared_ptr<UniformBuffer>>& uniformBuffers);

	void bindDescriptorSet(VkCommandBuffer commandBuffer, const Pipeline& pipeline);
	void updateDynamicOffsets(uint32_t index);

	VkDescriptorSetLayout getVkDescriptorSetLayout() const;
	VkDescriptorSet& getVkDescriptorSet(size_t i);

};