#pragma once

#include "physical_device/device/physical_device.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <string>
#include <stdexcept>

class PushConstants {
	std::vector<VkPushConstantRange> _ranges;
	uint32_t _count;

	uint32_t _maxSize;
public:
	PushConstants(const std::shared_ptr<PhysicalDevice>& physicalDevice) {
		auto limits = physicalDevice->getPhysicalDeviceLimits();

		_maxSize = limits.maxPushConstantsSize;
	}

	template<typename StructObject>
	void addPushConstant(VkShaderStageFlags shaderStages);

	const std::vector<VkPushConstantRange>& getVkPushConstantRange() const {
		return _ranges;
	}

	uint32_t getOffset(uint32_t index) const {
		return _ranges[index].offset;
	}

	uint32_t getSize(uint32_t index) const {
		return _ranges[index].size;
	}

	VkShaderStageFlags getVkShaderStageFlags(uint32_t index) const {
		return _ranges[index].stageFlags;
	}
};


template<typename StructObject>
void PushConstants::addPushConstant(VkShaderStageFlags shaderStages) {
	static uint32_t cumulatedOffset = {};
	uint32_t size = sizeof(StructObject);

	if (size + cumulatedOffset > _maxSize) {
		throw std::runtime_error("Push constants size exceeds the limit of " + std::to_string(_maxSize) + " bytes in size!");
	}

	_ranges.emplace_back(shaderStages, cumulatedOffset, size);
	cumulatedOffset += size;
}