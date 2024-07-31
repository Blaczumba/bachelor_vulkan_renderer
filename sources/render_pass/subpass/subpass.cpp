#include "subpass.h"

#include <stdexcept>

Subpass::Subpass(const AttachmentLayout& layout) : _layout(layout) {

}

VkSubpassDescription Subpass::getVkSubpassDescription() const {
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(_colorAttachmentRefs.size());
    subpass.inputAttachmentCount = static_cast<uint32_t>(_inputAttachmentRefs.size());
    if (!_inputAttachmentRefs.empty())
        subpass.pInputAttachments = _inputAttachmentRefs.data();
    if (!_colorAttachmentRefs.empty())
        subpass.pColorAttachments = _colorAttachmentRefs.data();
    if (!_depthAttachmentRefs.empty())
        subpass.pDepthStencilAttachment = _depthAttachmentRefs.data();
    if (!_colorAttachmentResolveRefs.empty())
        subpass.pResolveAttachments = _colorAttachmentResolveRefs.data();

    return subpass;
}

void Subpass::addSubpassOutputAttachment(uint32_t attachmentBinding, VkImageLayout layout) {
    const auto& attachments = _layout.getAttachments();
    if (attachments.size() <= attachmentBinding)
        throw std::runtime_error("attachmentBinding is not a valid index in attachments vector!");

    const Attachment attachment = attachments[attachmentBinding];

    VkAttachmentReference attachmentReference{};
    attachmentReference.layout = layout;
    attachmentReference.attachment = attachmentBinding;

    switch (attachment.getAttachmentRefType()) {
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

    VkAttachmentReference attachmentReference{};
    attachmentReference.layout = layout;
    attachmentReference.attachment = attachmentBinding;

    _inputAttachmentRefs.push_back(attachmentReference);
}
