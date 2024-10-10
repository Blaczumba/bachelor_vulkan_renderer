#pragma once

#include "render_pass/attachment/attachment.h"

#include <vulkan/vulkan.h>

#include <algorithm>
#include <iterator>
#include <vector>

class AttachmentLayout {
	std::vector<Attachment> _attachments;

public:
	void addAttachment(const Attachment& attachement) {
		_attachments.push_back(attachement);
	}

	const std::vector<Attachment>& getAttachments() const {
		return _attachments;
	}

	std::vector<VkClearValue> getVkClearValues() const {
		std::vector<VkClearValue> clearValues;
		std::transform(_attachments.cbegin(), _attachments.cend(), std::back_inserter(clearValues), [](const Attachment& attachment) { return attachment.getClearValue(); });
		return clearValues;
	}

	std::vector<VkAttachmentDescription> getVkAttachmentDescriptions() const {
		std::vector<VkAttachmentDescription> descriptions;
		std::transform(_attachments.cbegin(), _attachments.cend(), std::back_inserter(descriptions), [](const Attachment& attachment) { return attachment.getDescription(); });
		return descriptions;
	}

	uint32_t getColorAttachmentsCount() const {
		return std::count_if(_attachments.cbegin(), _attachments.cend(), [](const Attachment& attachment) { return attachment.getAttachmentRefType() == Attachment::Type::COLOR_ATTACHMENT; });
	}
};

class Subpass {
	const AttachmentLayout& _layout;

	std::vector<VkAttachmentReference> _inputAttachmentRefs;
	std::vector<VkAttachmentReference> _colorAttachmentRefs;
	std::vector<VkAttachmentReference> _depthAttachmentRefs;
	std::vector<VkAttachmentReference> _colorAttachmentResolveRefs;

public:
	Subpass(const AttachmentLayout& layout);
	void addSubpassOutputAttachment(uint32_t attachmentBinding, VkImageLayout layout);
	void addSubpassInputAttachment(uint32_t attachmentBinding, VkImageLayout layout);
	VkSubpassDescription getVkSubpassDescription() const;

	const std::vector<VkAttachmentReference>& getInputAttachmentRefs() const { return _inputAttachmentRefs; }
	const std::vector<VkAttachmentReference>& getColorAttachmentRefs() const { return _colorAttachmentRefs; }
	const std::vector<VkAttachmentReference>& getDepthAttachmentRefs() const { return _depthAttachmentRefs; }
	const std::vector<VkAttachmentReference>& getColorResolveAttachmentRefs() const { return _colorAttachmentResolveRefs; }
};
