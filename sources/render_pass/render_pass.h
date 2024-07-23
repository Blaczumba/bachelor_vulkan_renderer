#pragma once

#include "logical_device/logical_device.h"
#include "attachment/attachment.h"

#include <vector>
#include <variant>
#include <memory>

class AttachmentLayout {
	std::vector<Attachment> _attachments;
public:
	void addAttachment(const Attachment& attachement) {
		_attachments.push_back(attachement);
	}

	const std::vector<Attachment>& getAttachments() const {
		return _attachments;
	}
};

class Renderpass {
	VkRenderPass _renderpass;
	std::vector<VkClearValue> _clearValues;
	AttachmentLayout _attachmentsLayout;
	std::shared_ptr<LogicalDevice> _logicalDevice;
	uint32_t _colorAttachmentsCount;
public:
	Renderpass(std::shared_ptr<LogicalDevice> logicalDevice, const AttachmentLayout& layout);
	~Renderpass();
	VkRenderPass getVkRenderPass();
	const std::vector<VkClearValue>& getClearValues() const;
	const AttachmentLayout& getAttachmentsLayout() const;
	uint32_t getColorAttachmentsCount() const;
};
