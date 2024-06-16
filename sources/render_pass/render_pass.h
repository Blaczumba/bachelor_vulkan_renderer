#pragma once

#include "logical_device/logical_device.h"
#include "attachment/attachment.h"
#include "attachment/attachment_type.h"

#include <vector>
#include <variant>
#include <memory>

using Attachment = std::variant<ColorAttachment, DepthAttachment, ColorAttachmentResolve>;

class Renderpass {
	VkRenderPass _renderpass;
	std::vector<VkClearValue> _clearValues;
	std::vector<AttachmentType> _layout;
	std::shared_ptr<LogicalDevice> _logicalDevice;
public:
	Renderpass(std::shared_ptr<LogicalDevice> logicalDevice, const std::vector<Attachment>& attachments);
	VkRenderPass getVkRenderPass();
	const std::vector<VkClearValue>& getClearValues() const;
	const std::vector<AttachmentType>& getLayout() const;
};
