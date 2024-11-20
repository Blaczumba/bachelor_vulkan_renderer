#include "subpass.h"

#include "render_pass/attachment/attachment_layout.h"

#include <stdexcept>

Subpass::Subpass(const AttachmentLayout& layout) : _layout(layout) {}

VkSubpassDescription Subpass::getVkSubpassDescription() const {
    return VkSubpassDescription {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = static_cast<uint32_t>(_inputAttachmentRefs.size()),
        .pInputAttachments = !_inputAttachmentRefs.empty() ? _inputAttachmentRefs.data() : nullptr,
        .colorAttachmentCount = static_cast<uint32_t>(_colorAttachmentRefs.size()),
        .pColorAttachments = !_colorAttachmentRefs.empty() ? _colorAttachmentRefs.data() : nullptr,
        .pResolveAttachments = !_colorAttachmentResolveRefs.empty() ? _colorAttachmentResolveRefs.data() : nullptr,
        .pDepthStencilAttachment = !_depthAttachmentRefs.empty() ? _depthAttachmentRefs.data() : nullptr
    };
}

void Subpass::addSubpassOutputAttachment(uint32_t attachmentBinding) {
    const auto& attachments = _layout.getAttachments();
    if (attachments.size() <= attachmentBinding)
        throw std::runtime_error("attachmentBinding is not a valid index in attachments vector!");

    const Attachment attachment = attachments[attachmentBinding];

    const VkAttachmentReference attachmentReference = {
        .attachment = attachmentBinding,
        .layout = attachment.getSubpassImageLayout()
    };

    switch (attachment.getAttachmentType()) {
    case Attachment::Type::COLOR_ATTACHMENT:
        _colorAttachmentRefs.push_back(attachmentReference);
        break;
    case Attachment::Type::COLOR_ATTACHMENT_RESOLVE:
        _colorAttachmentResolveRefs.push_back(attachmentReference);
        break;
    case Attachment::Type::DEPTH_ATTACHMENT:
        _depthAttachmentRefs.push_back(attachmentReference);
        break;
    default:
        throw std::runtime_error("Unknown attachment type");
    }
}

void Subpass::addSubpassInputAttachment(uint32_t attachmentBinding, VkImageLayout layout) {
    const auto& attachments = _layout.getAttachments();
    if (attachments.size() <= attachmentBinding)
        throw std::runtime_error("attachmentBinding is not a valid index in attachments vector!");

    const VkAttachmentReference attachmentReference = {
        .attachment = attachmentBinding,
        .layout = layout
    };

    _inputAttachmentRefs.push_back(attachmentReference);
}
