#pragma once

#include "logical_device/logical_device.h"

#include <memory>

class Pipeline {
protected:
	VkPipeline _pipeline				= VK_NULL_HANDLE;
	VkPipelineLayout _pipelineLayout	= VK_NULL_HANDLE;
	VkPipelineBindPoint _pipelineBindPoint;

	const LogicalDevice& _logicalDevice;
public:
	Pipeline(const LogicalDevice& logicalDevice, VkPipelineBindPoint pipelineBindPoint);
	virtual ~Pipeline() = default;

	VkPipeline getVkPipeline() const;
	VkPipelineLayout getVkPipelineLayout() const;
	VkPipelineBindPoint getVkPipelineBindPoint() const;
};