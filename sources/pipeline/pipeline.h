#pragma once

#include "logical_device/logical_device.h"

#include <memory>

class Pipeline {
protected:
	VkPipeline _pipeline;
	VkPipelineLayout _pipelineLayout;

	std::shared_ptr<LogicalDevice> _logicalDevice;
public:
	Pipeline(std::shared_ptr<LogicalDevice> logicalDevice);
	virtual ~Pipeline() = default;

	VkPipeline getVkPipeline() const;
	VkPipelineLayout getVkPipelineLayout() const;
};