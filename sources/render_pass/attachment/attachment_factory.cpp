#include "attachment_factory.h"

Attachment AttachmentFactory::createColorAttachment(
    VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkSampleCountFlagBits samples) {
    return Attachment(
        Attachment::Type::COLOR_ATTACHMENT,
        VkClearValue{ .color = { 0.0f, 0.0f, 0.0f, 1.0f } },
        createDescription(format, samples, loadOp, storeOp, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    );
}

Attachment AttachmentFactory::createColorPresentAttachment(
    VkFormat format, VkAttachmentLoadOp loadOp) {
    return Attachment(
        Attachment::Type::COLOR_ATTACHMENT,
        VkClearValue{ .color = { 0.0f, 0.0f, 0.0f, 1.0f } },
        createDescription(format, VK_SAMPLE_COUNT_1_BIT, loadOp, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR),
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    );
}

Attachment AttachmentFactory::createDepthAttachment(
    VkFormat format, VkAttachmentStoreOp storeOp, VkSampleCountFlagBits samples,
    VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp) {
    return Attachment(
        Attachment::Type::DEPTH_ATTACHMENT,
        VkClearValue{ .depthStencil = { 1.0f, 0 } },
        createDescription(format, samples, VK_ATTACHMENT_LOAD_OP_CLEAR, storeOp, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, stencilLoadOp, stencilStoreOp),
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    );
}

Attachment AttachmentFactory::createShadowAttachment(
    VkFormat format, VkImageLayout finalLayout) {
    return Attachment(
        Attachment::Type::DEPTH_ATTACHMENT,
        VkClearValue{ .depthStencil = { 1.0f, 0 } },
        createDescription(format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_UNDEFINED, finalLayout),
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    );
}

Attachment AttachmentFactory::createColorResolveAttachment(
    VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp) {
    return Attachment(
        Attachment::Type::COLOR_ATTACHMENT_RESOLVE,
        VkClearValue{ .color = { 0.0f, 0.0f, 0.0f, 1.0f } },
        createDescription(format, VK_SAMPLE_COUNT_1_BIT, loadOp, storeOp, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    );
}

Attachment AttachmentFactory::createColorResolvePresentAttachment(
    VkFormat format, VkAttachmentLoadOp loadOp) {
    return Attachment(
        Attachment::Type::COLOR_ATTACHMENT_RESOLVE,
        VkClearValue{ .color = { 0.0f, 0.0f, 0.0f, 1.0f } },
        createDescription(format, VK_SAMPLE_COUNT_1_BIT, loadOp, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR),
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    );
}

VkAttachmentDescription AttachmentFactory::createDescription(
    VkFormat format,
    VkSampleCountFlagBits samples,
    VkAttachmentLoadOp loadOp,
    VkAttachmentStoreOp storeOp,
    VkImageLayout initialLayout,
    VkImageLayout finalLayout,
    VkAttachmentLoadOp stencilLoadOp,
    VkAttachmentStoreOp stencilStoreOp) {
    return VkAttachmentDescription{
        .flags = 0,
        .format = format,
        .samples = samples,
        .loadOp = loadOp,
        .storeOp = storeOp,
        .stencilLoadOp = stencilLoadOp,
        .stencilStoreOp = stencilStoreOp,
        .initialLayout = initialLayout,
        .finalLayout = finalLayout
    };
}