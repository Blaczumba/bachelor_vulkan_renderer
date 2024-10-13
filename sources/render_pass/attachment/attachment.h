#pragma once

#include <vulkan/vulkan.h>

struct Attachment {
public:
    enum class Type : uint8_t {
        COLOR_ATTACHMENT = 0,
        COLOR_ATTACHMENT_RESOLVE,
        DEPTH_ATTACHMENT
    };

    VkClearValue getClearValue() const { return clearValue; }
    const VkAttachmentDescription& getDescription() const { return description; }
    Type getAttachmentRefType() const { return type; }
    VkImageLayout getSubpassImageLayout() const { return subpassImageLayout; }

protected:
    VkClearValue clearValue{};
    VkAttachmentDescription description{};
    Type type{};
    VkImageLayout subpassImageLayout{};

    constexpr Attachment(Type type, VkClearValue clearValue, VkAttachmentDescription description, VkImageLayout subpassImageLayout)
        : type(type), clearValue(clearValue), description(description), subpassImageLayout(subpassImageLayout) {}
};

// Helper function for initializing common fields in VkAttachmentDescription
constexpr VkAttachmentDescription createDescription(
    VkFormat format,
    VkSampleCountFlagBits samples,
    VkAttachmentLoadOp loadOp,
    VkAttachmentStoreOp storeOp,
    VkImageLayout initialLayout,
    VkImageLayout finalLayout,
    VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE
) {
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

// Specific attachments below

struct ColorAttachment : public Attachment {
    constexpr ColorAttachment(VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT)
        : Attachment(
            Type::COLOR_ATTACHMENT,
            VkClearValue{ .color = { 0.0f, 0.0f, 0.0f, 1.0f } },
            createDescription(format, samples, loadOp, storeOp, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        ) {}
};

struct ColorPresentAttachment : public Attachment {
    constexpr ColorPresentAttachment(VkFormat format, VkAttachmentLoadOp loadOp)
        : Attachment(
            Type::COLOR_ATTACHMENT,
            VkClearValue{ .color = { 0.0f, 0.0f, 0.0f, 1.0f } },
            createDescription(format, VK_SAMPLE_COUNT_1_BIT, loadOp, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR),
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        ) {}
};

struct DepthAttachment : public Attachment {
    constexpr DepthAttachment(VkFormat format, VkAttachmentStoreOp storeOp, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE)
        : Attachment(
            Type::DEPTH_ATTACHMENT,
            VkClearValue{ .depthStencil = { 1.0f, 0 } },
            createDescription(format, samples, VK_ATTACHMENT_LOAD_OP_CLEAR, storeOp, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, stencilLoadOp, stencilStoreOp),
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        ) {}
};

struct ShadowAttachment : public Attachment {
    constexpr ShadowAttachment(VkFormat format, VkImageLayout finalLayout)
        : Attachment(
            Type::DEPTH_ATTACHMENT,
            VkClearValue{ .depthStencil = { 1.0f, 0 } },
            createDescription(format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_UNDEFINED, finalLayout),
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        ) {}
};

struct ColorResolveAttachment : public Attachment {
    constexpr ColorResolveAttachment(VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp)
        : Attachment(
            Type::COLOR_ATTACHMENT_RESOLVE,
            VkClearValue{ .color = { 0.0f, 0.0f, 0.0f, 1.0f } },
            createDescription(format, VK_SAMPLE_COUNT_1_BIT, loadOp, storeOp, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        ) {}
};

struct ColorResolvePresentAttachment : public Attachment {
    constexpr ColorResolvePresentAttachment(VkFormat format, VkAttachmentLoadOp loadOp)
        : Attachment(
            Type::COLOR_ATTACHMENT_RESOLVE,
            VkClearValue{ .color = { 0.0f, 0.0f, 0.0f, 1.0f } },
            createDescription(format, VK_SAMPLE_COUNT_1_BIT, loadOp, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR),
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        ) {}
};
