#pragma once

#include "logical_device/logical_device.h"
#include "attachment/attachment.h"

#include <vector>
#include <memory>

class Renderpass {
	VkRenderPass _renderpass;
	std::shared_ptr<LogicalDevice> _logicalDevice;
public:
	Renderpass(std::shared_ptr<LogicalDevice> logicalDevice, const std::vector<Attachment>& attachments);
};
