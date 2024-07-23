#pragma once

#include <vulkan/vulkan.h>

struct Attachment {
public:
    enum class Type : uint8_t {
        COLOR_ATTACHMENT = 0,
        COLOR_ATTACHMENT_RESOLVE,
        DEPTH_ATTACHMENT
    };

    VkImageLayout getLayout() const { return layout; };
    VkClearValue getClearValue() const { return clearValue; };
    const VkAttachmentDescription& getDescription() const { return description; }
    Type getAttachmentRefType() const { return type; }

    VkSampleCountFlagBits getMsaaSampleCount() const { return description.samples; }

    virtual ~Attachment() = default;
protected:
    VkImageLayout layout;
    VkClearValue clearValue;
    VkAttachmentDescription description{};
    Type type;
};

struct ColorAttachment : public Attachment {
    ColorAttachment(VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT) {
        description.format = format;
        description.samples = samples;
        description.loadOp = loadOp;
        description.storeOp = storeOp;
        description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        description.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VK_IMAGE_LAYOUT_UNDEFINED
        description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        type = Type::COLOR_ATTACHMENT;
        clearValue = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    }
};

struct ColorPresentAttachment : public Attachment {
    ColorPresentAttachment(VkFormat format, VkAttachmentLoadOp loadOp) {
        description.format = format;
        description.samples = VK_SAMPLE_COUNT_1_BIT;
        description.loadOp = loadOp;
        description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        type = Type::COLOR_ATTACHMENT;
        clearValue = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    }
};

struct DepthAttachment : public Attachment {
    DepthAttachment(VkFormat format, VkAttachmentStoreOp storeOp, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE) {
        description.format = format;
        description.samples = samples;
        description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        description.storeOp = storeOp;
        description.stencilLoadOp = stencilLoadOp;
        description.stencilStoreOp = stencilStoreOp;
        description.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // VK_IMAGE_LAYOUT_UNDEFINED
        description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        type = Type::DEPTH_ATTACHMENT;
        clearValue = { 1.0f, 0 };
    }
};

struct ColorResolveAttachment : public Attachment {
    ColorResolveAttachment(VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp) {
        description.format = format;
        description.samples = VK_SAMPLE_COUNT_1_BIT;
        description.loadOp = loadOp;
        description.storeOp = storeOp;
        description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        description.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VK_IMAGE_LAYOUT_UNDEFINED
        description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        type = Type::COLOR_ATTACHMENT_RESOLVE;
        clearValue = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    }
};

struct ColorResolvePresentAttachment : public Attachment {
    ColorResolvePresentAttachment(VkFormat format, VkAttachmentLoadOp loadOp) {
        description.format = format;
        description.samples = VK_SAMPLE_COUNT_1_BIT;
        description.loadOp = loadOp;
        description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        type = Type::COLOR_ATTACHMENT_RESOLVE;
        clearValue = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    }
};
