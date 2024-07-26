#pragma once

#include "logical_device/logical_device.h"

#include <memory>

class Pipeline {
protected:
	VkPipeline _pipeline;
	VkPipelineLayout _pipelineLayout;
	VkPipelineBindPoint _pipelineBindPoint;

	std::shared_ptr<LogicalDevice> _logicalDevice;
public:
	Pipeline(std::shared_ptr<LogicalDevice> logicalDevice, VkPipelineBindPoint pipelineBindPoint);
	virtual ~Pipeline() = default;

	VkPipeline getVkPipeline() const;
	VkPipelineLayout getVkPipelineLayout() const;
	VkPipelineBindPoint getVkPipelineBindPoint() const;
};