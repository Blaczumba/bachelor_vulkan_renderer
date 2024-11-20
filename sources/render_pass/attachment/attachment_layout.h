#pragma once

#include <vulkan/vulkan.h>

#include <vector>

class Attachment;

class AttachmentLayout {
public:
	void addAttachment(const Attachment& attachement);
	const std::vector<Attachment>& getAttachments() const;
	const std::vector<VkClearValue>& getVkClearValues() const;
	uint32_t getAttachmentsCount() const;
	uint32_t getColorAttachmentsCount() const;

private:
	std::vector<Attachment> _attachments;
	std::vector<VkClearValue> _clearValues;
	uint32_t _colorAttachments{};
};
