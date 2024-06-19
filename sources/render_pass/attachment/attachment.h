#pragma once

#include <vulkan/vulkan.h>

#include <variant>

struct Attachment {
    virtual VkImageLayout getLayout() const { return layout; };

    const VkAttachmentDescription& getDescription() const { return description; }
    VkFormat getFromat() const { return description.format; }
    VkSampleCountFlagBits getMsaaSampleCount() const { return description.samples; }

    VkClearValue getClearValue() const { return clearValue; };
protected:
    VkImageLayout layout;
    VkClearValue clearValue;
    VkAttachmentDescription description{};
};

struct ColorAttachment : public Attachment {
    ColorAttachment(VkFormat format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT) {
        description.format = format;
        description.samples = samples;
        description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        description.finalLayout = (samples == VK_SAMPLE_COUNT_1_BIT) ?
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR :
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


        layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        clearValue = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    }
};

struct DepthAttachment : public Attachment {
    DepthAttachment(VkFormat format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT) {
        description.format = format;
        description.samples = samples;
        description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        clearValue = { 1.0f, 0 };
    }
};

struct ColorResolveAttachment : public Attachment {
    ColorResolveAttachment(VkFormat format) {
        description.format = format;
        description.samples = VK_SAMPLE_COUNT_1_BIT;
        description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        clearValue = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    }
};
