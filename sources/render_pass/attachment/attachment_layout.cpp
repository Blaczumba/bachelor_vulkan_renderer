#include "attachment_layout.h"

#include "attachment.h"

#include <algorithm>
#include <iterator>

void AttachmentLayout::addAttachment(const Attachment& attachement) {
	_attachments.push_back(attachement);
}

const std::vector<Attachment>& AttachmentLayout::getAttachments() const {
	return _attachments;
}

std::vector<VkClearValue> AttachmentLayout::getVkClearValues() const {
	std::vector<VkClearValue> clearValues;
	std::transform(_attachments.cbegin(), _attachments.cend(), std::back_inserter(clearValues), [](const Attachment& attachment) { return attachment.getClearValue(); });
	return clearValues;
}

std::vector<VkAttachmentDescription> AttachmentLayout::getVkAttachmentDescriptions() const {
	std::vector<VkAttachmentDescription> descriptions;
	std::transform(_attachments.cbegin(), _attachments.cend(), std::back_inserter(descriptions), [](const Attachment& attachment) { return attachment.getDescription(); });
	return descriptions;
}

uint32_t AttachmentLayout::getColorAttachmentsCount() const {
	return std::count_if(_attachments.cbegin(), _attachments.cend(), [](const Attachment& attachment) { return attachment.getAttachmentType() == Attachment::Type::COLOR_ATTACHMENT; });
}