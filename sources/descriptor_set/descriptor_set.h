#pragma once

#include <logical_device/logical_device.h>
#include <memory_objects/uniform_buffer.h>

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

struct DescriptorSetLayoutElement {
	VkDescriptorType type;
	VkShaderStageFlags stage;
};

class DescriptorSetLayout {
	VkDescriptorSetLayout _layout;
	std::vector<DescriptorSetLayoutElement> _layoutElements;

	std::shared_ptr<LogicalDevice> _logicalDevice;
public:
	DescriptorSetLayout(std::shared_ptr<LogicalDevice> logicalDevice,  const std::vector<DescriptorSetLayoutElement>& layoutElements);
	~DescriptorSetLayout();
};

class DescriptorSet {
	DescriptorSetLayout _layout;
public:
	DescriptorSet(const DescriptorSetLayout& layout);
};