#pragma once

#include "attachment.h"

#include <vulkan/vulkan.h>

class AttachmentFactory {
public:
    static Attachment createColorAttachment(
        VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);

    static Attachment createColorPresentAttachment(
        VkFormat format, VkAttachmentLoadOp loadOp);

    static Attachment createDepthAttachment(
        VkFormat format, VkAttachmentStoreOp storeOp, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
        VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE);

    static Attachment createShadowAttachment(
        VkFormat format, VkImageLayout finalLayout);

    static Attachment createColorResolveAttachment(
        VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp);

    static Attachment createColorResolvePresentAttachment(
        VkFormat format, VkAttachmentLoadOp loadOp);

private:
    static VkAttachmentDescription createDescription(
        VkFormat format,
        VkSampleCountFlagBits samples,
        VkAttachmentLoadOp loadOp,
        VkAttachmentStoreOp storeOp,
        VkImageLayout initialLayout,
        VkImageLayout finalLayout,
        VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE);
};
