#pragma once

#include "logical_device/logical_device.h"
#include "attachment/attachment.h"

#include <vector>
#include <variant>
#include <memory>

using Attachment = std::variant<ColorAttachment, DepthAttachment, ColorAttachmentResolve>;

class Renderpass {
	VkRenderPass _renderpass;
	std::vector<VkClearValue> _clearValues;
	std::vector<AttachmentElement> _layout;
	std::shared_ptr<LogicalDevice> _logicalDevice;
public:
	Renderpass(std::shared_ptr<LogicalDevice> logicalDevice, const std::vector<Attachment>& attachments);
	~Renderpass();
	VkRenderPass getVkRenderPass();
	const std::vector<VkClearValue>& getClearValues() const;
	const std::vector<AttachmentElement>& getLayout() const;
};
