#include "attachment_layout.h"

#include "attachment.h"

#include <algorithm>
#include <iterator>

void AttachmentLayout::addAttachment(const Attachment& attachement) {
	_attachments.push_back(attachement);
	if (attachement.getAttachmentType() == Attachment::Type::COLOR_ATTACHMENT)
		++_colorAttachments;
	_clearValues.push_back(attachement.getVkClearValue());
}

const std::vector<Attachment>& AttachmentLayout::getAttachments() const {
	return _attachments;
}

const std::vector<VkClearValue>& AttachmentLayout::getVkClearValues() const {
	return _clearValues;
}

uint32_t AttachmentLayout::getAttachmentsCount() const {
	return _attachments.size();
}

uint32_t AttachmentLayout::getColorAttachmentsCount() const {
	return _colorAttachments;
}