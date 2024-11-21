#include "framebuffer.h"

#include "logical_device/logical_device.h"
#include "render_pass/render_pass.h"
#include "swapchain/swapchain.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>

Framebuffer::Framebuffer(const Renderpass& renderpass, const Swapchain& swapchain, uint8_t swapchianIndex, const CommandPool& commandPool)
    : _renderpass(renderpass) {
    if (swapchain.getImagesCount() <= swapchianIndex)
        throw std::runtime_error("swapchainIndex does not fit in swapchain images count!");

    const std::vector<Attachment>& attachments = _renderpass.getAttachmentsLayout().getAttachments();
    std::vector<VkAttachmentDescription> descriptions;
    std::transform(attachments.cbegin(), attachments.cend(), std::back_inserter(descriptions), [](const Attachment& attachment) { return attachment.getDescription(); });

    std::vector<VkImageView> imageViews;
    const VkExtent2D swapchainExtent = swapchain.getExtent();
    for (const auto& description : descriptions) {
        switch (description.finalLayout) {
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            imageViews.push_back(swapchain.getVkImageViews()[swapchianIndex]);
            continue;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            _textureAttachments.emplace_back(TextureFactory::createColorAttachment(commandPool, description.format, description.samples, swapchainExtent));
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            _textureAttachments.emplace_back(TextureFactory::createDepthAttachment(commandPool, description.format, description.samples, swapchainExtent));
            break;
        default:
            throw std::runtime_error("failed to recognize final layout in the framebuffer!");
        }
        imageViews.push_back(_textureAttachments.back()->getVkImageView());
    }

    VkFramebufferCreateInfo framebufferInfo = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = _renderpass.getVkRenderPass(),
        .attachmentCount = static_cast<uint32_t>(imageViews.size()),
        .pAttachments = imageViews.data(),
        .width = swapchainExtent.width,
        .height = swapchainExtent.height,
        .layers = 1,
    };

    if (vkCreateFramebuffer(_renderpass.getLogicalDevice().getVkDevice(), &framebufferInfo, nullptr, &_framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

Framebuffer::Framebuffer(const Renderpass& renderpass, VkExtent2D extent, const std::vector<VkImageView>& imageViews)
    : _renderpass(renderpass) {
    VkFramebufferCreateInfo framebufferInfo = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = _renderpass.getVkRenderPass(),
        .attachmentCount = static_cast<uint32_t>(imageViews.size()),
        .pAttachments = imageViews.data(),
        .width = extent.width,
        .height = extent.height,
        .layers = 1,
    };

    if (vkCreateFramebuffer(_renderpass.getLogicalDevice().getVkDevice(), &framebufferInfo, nullptr, &_framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

Framebuffer::~Framebuffer() {
    VkDevice device = _renderpass.getLogicalDevice().getVkDevice();

    vkDestroyFramebuffer(device, _framebuffer, nullptr);
}

VkFramebuffer Framebuffer::getVkFramebuffer() const {
    return _framebuffer;
}
