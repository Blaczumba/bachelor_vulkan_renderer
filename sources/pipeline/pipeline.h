#pragma once

#include "logical_device/logical_device.h"

#include <memory>

class Pipeline {
protected:
	std::shared_ptr<LogicalDevice> _logicalDevice;
public:
	Pipeline(std::shared_ptr<LogicalDevice> logicalDevice);
	~Pipeline() = default;
};