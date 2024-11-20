#pragma once

#include <vulkan/vulkan.h>

#include <vector>

class Attachment;

class AttachmentLayout {
public:
	void addAttachment(const Attachment& attachement);

	const std::vector<Attachment>& getAttachments() const;
	std::vector<VkClearValue> getVkClearValues() const;

	std::vector<VkAttachmentDescription> getVkAttachmentDescriptions() const;

	uint32_t getColorAttachmentsCount() const;

private:
	std::vector<Attachment> _attachments;
};
