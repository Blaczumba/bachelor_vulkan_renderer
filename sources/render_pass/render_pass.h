#pragma once

#include "logical_device/logical_device.h"
#include "attachment/attachment.h"

#include <vector>
#include <variant>
#include <memory>

class Renderpass {
	VkRenderPass _renderpass;
	std::vector<VkClearValue> _clearValues;
	std::vector<std::unique_ptr<Attachment>> _attachments;
	std::shared_ptr<LogicalDevice> _logicalDevice;
	uint32_t _colorAttachmentsCount;
public:
	Renderpass(std::shared_ptr<LogicalDevice> logicalDevice, std::vector<std::unique_ptr<Attachment>>&& attachments);
	~Renderpass();
	VkRenderPass getVkRenderPass();
	const std::vector<VkClearValue>& getClearValues() const;
	const std::vector<std::unique_ptr<Attachment>>& getAttachments() const;
	uint32_t getColorAttachmentsCount() const;
};
